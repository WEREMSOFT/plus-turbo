#include "process.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>

// ------------------------------------------------------------
// Internal helpers
// ------------------------------------------------------------

static void mi_add_kv(mi_result_t *r, const char *key, const char *val) {
    if (r->kv_count >= MI_MAX_KV) return;

    strncpy(r->kv[r->kv_count].key, key, sizeof(r->kv[0].key) - 1);
    strncpy(r->kv[r->kv_count].value, val, sizeof(r->kv[0].value) - 1);
    r->kv_count++;
}

// ------------------------------------------------------------
// Minimal MI parser (state machine)
// ------------------------------------------------------------

typedef enum {
    S_START,
    S_CLASS,
    S_KEY,
    S_VALUE,
    S_STRING,
    S_ESCAPE
} mi_state_t;

static int mi_parse_line(const char *line, mi_result_t *out) {
    memset(out, 0, sizeof(*out));

    mi_state_t state = S_START;

    char key[64] = {0};
    char val[256] = {0};

    int ki = 0, vi = 0;

    const char *p = line;

    while (*p) {
        char c = *p;

        switch (state) {

        case S_START:
            if (c == '^' || c == '*' || c == '=' || c == '~' || c == '&') {
                out->type = c;
                state = S_CLASS;
            }
            break;

        case S_CLASS:
            if (c == ',') {
                out->klass[ki] = 0;
                ki = 0;
                state = S_KEY;
            } else if (c == '\n') {
                out->klass[ki] = 0;
                return 1;
            } else {
                if (ki < (int)sizeof(out->klass) - 1)
                    out->klass[ki++] = c;
            }
            break;

        case S_KEY:
            if (c == '=') {
                key[ki] = 0;
                ki = 0;
                state = S_VALUE;
            } else if (c == ',') {
                ki = 0;
            } else {
                if (ki < (int)sizeof(key) - 1)
                    key[ki++] = c;
            }
            break;

        case S_VALUE:
            if (c == '"') {
                vi = 0;
                state = S_STRING;
            } else {
                vi = 0;
                val[vi++] = c;
                state = S_STRING;
            }
            break;

        case S_STRING:
            if (c == '\\') {
                state = S_ESCAPE;
            } else if (c == '"') {
                val[vi] = 0;
                mi_add_kv(out, key, val);
                state = S_KEY;
                ki = 0;
            } else if (c == ',') {
                val[vi] = 0;
                mi_add_kv(out, key, val);
                state = S_KEY;
                ki = 0;
                vi = 0;
            } else {
                if (vi < (int)sizeof(val) - 1)
                    val[vi++] = c;
            }
            break;

        case S_ESCAPE:
            if (vi < (int)sizeof(val) - 1)
                val[vi++] = c;
            state = S_STRING;
            break;
        }

        p++;
    }

    return 1;
}

// ------------------------------------------------------------
// Buffer helpers
// ------------------------------------------------------------

static int mi_buffer_feed(mi_buffer_t *b, const char *input, int n) {
    if (b->len + n >= MI_BUF) return -1;

    memcpy(b->data + b->len, input, n);
    b->len += n;
    return 0;
}

static int mi_buffer_getline(mi_buffer_t *b, char *out, int max) {
    for (int i = 0; i < b->len; i++) {
        if (b->data[i] == '\n') {
            int len = i + 1;
            if (len >= max) len = max - 1;

            memcpy(out, b->data, len);
            out[len] = 0;

            memmove(b->data, b->data + i + 1, b->len - i - 1);
            b->len -= (i + 1);

            return 1;
        }
    }
    return 0;
}

// ------------------------------------------------------------
// Process lifecycle
// ------------------------------------------------------------

process_t process_start(const char *path, char *const argv[]) {
    process_t p;
    memset(&p, 0, sizeof(p));

    p.pid = -1;
    p.in_fd = -1;
    p.out_fd = -1;

    int in_pipe[2];
    int out_pipe[2];

    if (pipe(in_pipe) == -1) return p;
    if (pipe(out_pipe) == -1) return p;

    pid_t pid = fork();
    if (pid == -1) return p;

    if (pid == 0) {
        // Child
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(out_pipe[1], STDERR_FILENO);

        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);

        prctl(PR_SET_PDEATHSIG, SIGTERM);

        char *gdb_argv[] = {
            (char *)"gdb",
            (char *)"--interpreter=mi2",
            (char *)"--quiet",
            (char *)path,
            NULL
        };

        execvp("gdb", gdb_argv);
        perror("execvp failed");
        exit(1);
    }

    // Parent
    close(in_pipe[0]);
    close(out_pipe[1]);

    p.pid = pid;
    p.in_fd = in_pipe[1];
    p.out_fd = out_pipe[0];
    p.is_running = 1;

    fcntl(p.out_fd, F_SETFL, O_NONBLOCK);

    return p;
}

void process_kill(process_t *p) {
    if (p->pid > 0) {
        process_cmd(p, "-gdb-exit");

        kill(p->pid, SIGTERM);
        waitpid(p->pid, NULL, 0);

        if (p->in_fd >= 0) close(p->in_fd);
        if (p->out_fd >= 0) close(p->out_fd);

        p->pid = -1;
        p->in_fd = -1;
        p->out_fd = -1;
        p->is_running = 0;
    }
}

int process_is_running(process_t *p) {
    if (p->pid <= 0) return 0;

    int status;
    pid_t r = waitpid(p->pid, &status, WNOHANG);
    if (r == 0) return 1;

    p->is_running = 0;
    return 0;
}

// ------------------------------------------------------------
// Raw I/O
// ------------------------------------------------------------

int process_poll_output(process_t *p, char *buffer, size_t size) {
    ssize_t n = read(p->out_fd, buffer, size - 1);
    if (n > 0) {
        buffer[n] = 0;
        return (int)n;
    }

    if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return 0;

    return -1;
}

// ------------------------------------------------------------
// Command interface
// ------------------------------------------------------------

int process_cmd(process_t *p, const char *cmd) {
    if (p->in_fd < 0) return -1;

    write(p->in_fd, cmd, strlen(cmd));
    write(p->in_fd, "\n", 1);
    return 0;
}

// ------------------------------------------------------------
// Debugger helpers
// ------------------------------------------------------------

int process_run(process_t *p) {
    return process_cmd(p, "-exec-run");
}

int process_continue(process_t *p) {
    return process_cmd(p, "-exec-continue");
}

int process_step_into(process_t *p) {
    return process_cmd(p, "-exec-step");
}

int process_step_over(process_t *p) {
    return process_cmd(p, "-exec-next");
}

int process_pause(process_t *p) {
    return process_cmd(p, "-exec-interrupt");
}

int process_break(process_t *p, const char *location) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "-break-insert %s", location);
    return process_cmd(p, cmd);
}

int process_eval(process_t *p, const char *expr) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "-data-evaluate-expression %s", expr);
    return process_cmd(p, cmd);
}

int process_locals(process_t *p) {
    return process_cmd(p, "-stack-list-variables --simple-values");
}

int process_stack(process_t *p) {
    return process_cmd(p, "-stack-list-frames");
}

int process_frame_select(process_t *p, int level) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "-stack-select-frame %d", level);
    return process_cmd(p, cmd);
}

int process_read_memory(process_t *p, const char *addr, int count) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "-data-read-memory-bytes %s %d", addr, count);
    return process_cmd(p, cmd);
}

// ------------------------------------------------------------
// High-level result extraction
// ------------------------------------------------------------

int process_next_result(process_t *p, mi_result_t *out) {
    char tmp[512];
    char line[512];

    int n = process_poll_output(p, tmp, sizeof(tmp));
    if (n > 0) {
        mi_buffer_feed(&p->mi_buf, tmp, n);
    }

    if (mi_buffer_getline(&p->mi_buf, line, sizeof(line))) {
        return mi_parse_line(line, out);
    }

    return 0;
}