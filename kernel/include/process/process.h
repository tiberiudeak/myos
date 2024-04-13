#ifndef _PROCESS_H
#define _PROCESS_H 1

#include <stdint.h>

typedef enum {
    TASK_CREATED,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
} task_state_t;

typedef struct {
    uint32_t task_id;
    task_state_t state;
    void *exec_address;
    int argc;
    char **argv;
} task_struct;

void enter_usermode(uint32_t, uint32_t);
task_struct *create_task(void *, int, char**);
void destroy_task(task_struct *);

#endif

