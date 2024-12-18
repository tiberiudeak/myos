#ifndef _PROCESS_H
#define _PROCESS_H 1

#include <stdint.h>
#include <mm/vmm.h>

enum task_state {
    TASK_CREATED,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
};

// context of a running process
struct proc_context {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t cs;
    uint32_t flags;
    uint32_t ss;
};

struct mapping {
    void *address;
    uint32_t size;
    struct mapping *next;
};

// task
struct task_struct {
    uint32_t task_id;
    enum task_state state;
    void *exec_address;
    int argc;
    char **argv;
    struct page_directory *vas;
    struct proc_context *context;
    void *heap_start;
    void *program_break;
    uint32_t heap_size_blocks;
    struct mapping *maps;
	uint32_t run_time;
	uint32_t sleep_time;
};

struct task_struct *create_task(void *, int, char**, int);
void destroy_task(struct task_struct *);

#ifdef CONFIG_FCFS_SCH
    void enter_usermode(uint32_t, uint32_t);
#endif

#endif /* !_PROCESS_H */

