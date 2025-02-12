#ifndef _PROCESS_H
#define _PROCESS_H 1

#include <mm/vmm.h>

#include <stdint.h>

#define KSTACK_SIZE 4096

enum task_state {
	TASK_CREATED,
	TASK_READY,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_TERMINATED
};

/*
 * context of a running process
 *
 * WARNING: do not change the order of the fields
 * (USERESP should be at offset 0x40 and SS at 0x3c)
 *
 * if order is modified, also update isr_func.S and
 * irq_funcs.S
 */
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
	uint32_t useresp;
};

struct mapping {
	void *address;
	uint32_t size;
	struct mapping *next;
};

/**
 * task struct
 *
 * WARNING: do not change the order of the fields
 * until the kstack (the context should remain at
 * offset 0x18 and kstack at 0x1c)
 *
 * if order is modified, also update irq_funcs.S and
 * isr_funcs.S
 */
struct task_struct {
	uint32_t task_id;
	enum task_state state;
	void *exec_address;
	int argc;
	char **argv;
	struct page_directory *vas;
	struct proc_context *context;
	void *kstack;
	void *heap_start;
	void *program_break;
	uint32_t heap_size_blocks;
	struct mapping *maps;
	uint32_t run_time;
	uint32_t sleep_time;
	int ring;
};

struct task_struct *create_task(void *, int, char **, int);
void destroy_task(struct task_struct *);
void ktask_exit(void);

#ifdef CONFIG_FCFS_SCH
void enter_usermode(uint32_t, uint32_t);
#endif

#endif /* !_PROCESS_H */
