#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MI_MAX_KV 32
#define MI_BUF 4096

typedef struct {
    char key[64];
    char value[256];
} mi_kv_t;

typedef struct {
    char type;              // '^', '*', '=', '~', '&'
    char klass[32];         // done, stopped, running, etc

    mi_kv_t kv[MI_MAX_KV];
    int kv_count;
} mi_result_t;

typedef struct {
    char data[MI_BUF];
    int len;
} mi_buffer_t;

typedef struct {
    pid_t pid;

    int in_fd;     // write commands to GDB
    int out_fd;    // read GDB output

    int is_running;

    mi_buffer_t mi_buf;
} process_t;


// --- Process lifecycle ---

process_t process_start(const char *path, char *const argv[]);
void process_kill(process_t *p);
int process_is_running(process_t *p);

// --- Raw I/O ---

int process_poll_output(process_t *p, char *buffer, size_t size);

// --- Debugger control ---

int process_cmd(process_t *p, const char *cmd);

int process_run(process_t *p);
int process_continue(process_t *p);
int process_step_into(process_t *p);
int process_step_over(process_t *p);
int process_pause(process_t *p);
int process_break(process_t *p, const char *location);

// --- Inspection ---

int process_eval(process_t *p, const char *expr);
int process_locals(process_t *p);
int process_stack(process_t *p);
int process_frame_select(process_t *p, int level);
int process_read_memory(process_t *p, const char *addr, int count);

// --- MI parsing ---

int process_next_result(process_t *p, mi_result_t *out);

#ifdef __cplusplus
}
#endif

#endif