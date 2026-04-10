#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
    pid_t pid;
    int stdout_fd;
} process_t;

/**
 * Starts a process and captures its stdout, forcing line buffering via stdbuf.
 * The child will automatically receive SIGTERM if the parent dies.
 * 
 * Usage: process_t p = process_start("./my_prog", "arg1", "arg2", NULL);
 * 
 * @param path The executable path or name.
 * @param ... NULL-terminated list of arguments (optional).
 * @return process_t structure. pid will be -1 on failure.
 */
static inline process_t process_start(const char *path, ...) {
    process_t p = { .pid = -1, .stdout_fd = -1 };
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        return p;
    }

    // Prepare argv array from variadic arguments
    va_list args;
    va_start(args, path);
    int count = 0;
    va_list args_copy;
    va_copy(args_copy, args);
    while (va_arg(args_copy, char *)) count++;
    va_end(args_copy);

    // We need space for: "stdbuf", "-oL", path, [count args], NULL
    char **argv = (char **)malloc((count + 4) * sizeof(char *));
    if (!argv) {
        va_end(args);
        close(pipefd[0]);
        close(pipefd[1]);
        return p;
    }

    argv[0] = "stdbuf";
    argv[1] = "-oL";
    argv[2] = (char *)path;
    for (int i = 0; i < count; i++) {
        argv[i + 3] = va_arg(args, char *);
    }
    argv[count + 3] = NULL;
    va_end(args);

    pid_t pid = fork();
    if (pid == -1) {
        free(argv);
        close(pipefd[0]);
        close(pipefd[1]);
        return p;
    }

    if (pid == 0) { // Child
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Ensure child (stdbuf) and its subsequent exec'd process die if parent dies
        // PR_SET_PDEATHSIG is preserved across execve (unless set-uid etc)
        prctl(PR_SET_PDEATHSIG, SIGTERM);

        execvp("stdbuf", argv);
        
        // If execvp returns, it failed
        perror("process_start: execvp failed");
        exit(1);
    }

    // Parent
    free(argv);
    close(pipefd[1]);

    // Set non-blocking read
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    p.pid = pid;
    p.stdout_fd = pipefd[0];
    return p;
}

/**
 * Polls for output. Returns bytes read, 0 if no data, -1 on EOF/Error.
 */
static inline int process_poll_output(process_t *p, char *buffer, size_t buffer_size) {
    if (p->stdout_fd < 0) return -1;
    ssize_t n = read(p->stdout_fd, buffer, buffer_size - 1);
    if (n > 0) {
        buffer[n] = '\0';
        return (int)n;
    }
    if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
    return -1;
}

/**
 * Explicitly kills the process.
 */
static inline void process_kill(process_t *p) {
    if (p->pid > 0) {
        kill(p->pid, SIGTERM);
        int status;
        waitpid(p->pid, &status, 0);
        if (p->stdout_fd >= 0) close(p->stdout_fd);
        p->pid = -1;
        p->stdout_fd = -1;
    }
}

#endif // PROCESS_H
