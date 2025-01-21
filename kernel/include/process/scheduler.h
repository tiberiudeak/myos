#ifndef _SCH_H
#define _SCH_H 1

#include <arch/i386/isr.h>
#include <kernel/list.h>
#include <process/process.h>

#include <stdint.h>

// node in the task queue
struct task_node {
	struct task_struct *task;
	struct embedded_link list;
};

/*
 * delta queue for sleeping tasks
 *
 * tasks are sorted by the difference in sleeping
 * times relative to the previous task
 *
 * e.g. task1 sleeps 10ms, task2 12ms and task3 2ms:
 *
 *          -------    -------    -------
 * head ->  | 2ms | -> | 8ms | -> | 2ms |
 *          -------    -------    -------
 *           task3      task1      task2
 *
 * the head represents the next task to wake up, with the
 * smallest remaining time
 */
struct delta_queue_node {
	int delta_time_ms;
	struct task_struct *task;
	struct embedded_link list;
};

typedef enum { RUNNING_TASK_QUEUE, SLEEPING_TASK_QUEUE } QUEUE_TYPE;

/*
 * Problems that might appear when switching tasks
 *
 * MANUAL_PUSH (ring0 -> ring3)
 *
 * the manual push problem appears when the interrupted task was running
 * in ring 0 and the new scheduled task will run in ring 3. When the running
 * task in ring 0 is interrupted, the CPU pushes the EFLAGS, CS and EIP
 * registers on the stack before entering the interrupt handler (because there
 * is no change in privilege level). After the new task has been scheduled, the
 * IRET instruction right at the end of the interrupt handler will pop out from
 * the stack EIP, CS, EFLAGS, ESP and SS (new task runs in user space and in
 * this case there is a privilege change). The handler will manually make space
 * on the stack and put the ESP and SS corresponding to the new task.
 *
 *
 * stack at the beginning				stack right before IRET
 *	  of int handler
 *
 * |         ...         |				|         ...         |
 * |---------------------|				|---------------------|
 * |        EFLAGS       |				|         SS          |
 * |---------------------|				|---------------------|
 * |         CS          |				|       USERESP       |
 * |---------------------|				|---------------------|
 * |         EIP         |				|        EFLAGS       |
 * |---------------------|				|---------------------|
 * |         ...         |				|         CS          |
 * |                     |				|---------------------|
 *                                      |         EIP         |
 *                                      |---------------------|
 *                                      |         ...         |
 *
 *
 * MANUAL POP (ring3 -> ring0)
 *
 * the manual pop problem appears when the interrupted task was running
 * in ring 3 and the new scheduled tasj will run in ring 0. When the running
 * task in ring 3 is interrupted, the CPU pushes the SS, USERESP, EFLAGS,
 * CS and EIP on the stack before entering the interrupt handler (change in
 * privilege level). After the new tack has been scheduled, the IRET
 * instruction right at the end of the interrupt handler will pop out from
 * the stack EIP, CS and EFLAGS (new task runs in ring 0, so no privilege
 * change). The handler will manually pop out from the stack the unused
 * USERESP and SS.
 *
 *
 * stack at the beginning				stack right before IRET
 *	  of int handler
 *
 * |         ...         |				|         ...         |
 * |---------------------|				|---------------------|
 * |         SS          |				|       EFLAGS        |
 * |---------------------|				|---------------------|
 * |       USERESP       |				|         CS          |
 * |---------------------|				|---------------------|
 * |        EFLAGS       |				|         EIP         |
 * |---------------------|				|---------------------|
 * |         CS          |				|         ...         |
 * |---------------------|
 * |         EIP         |
 * |---------------------|
 * |         ...         |
 *
 *
 * CHANGE KSTACK
 *
 * the change kstack situation appears when the new scheduled task will run
 * in ring 0 and has no prior context (first time running). The handler will
 * change to the kernel stack corresponding to this task before performing
 * the IRET.
 *
 * RESUME_KSTACK
 *
 * the resume kstack situation appears when the new scheduled task will run
 * in ring 0 and has prior context. The handler will restore the kernel stack
 * state before performing the IRET.
 */
typedef enum {
	NO_PROBLEM = 0x0,
	MANUAL_PUSH = 0x1,
	MANUAL_POP = 0x2,
	CHANGE_KSTACK = 0x4,
	RESUME_KSTACK = 0x8
} TASK_SWITCH_STACK_PROBLEM;

void enqueue_task(struct task_struct *);
struct task_struct *dequeue_task(void);

#ifdef CONFIG_FCFS_SCH
uint8_t scheduler_init(void);
uint8_t init_task_queue(void);
void simple_task_scheduler(void);
#else
uint8_t scheduler_init_rr(void);
uint32_t queue_size(struct embedded_link *);
void schedule(QUEUE_TYPE);
void start_init_task(void);
void change_context(struct interrupt_regs *, TASK_SWITCH_STACK_PROBLEM);
void resume_context(struct interrupt_regs *, TASK_SWITCH_STACK_PROBLEM);
void change_context_kernel(struct interrupt_regs *);
void display_running_processes(void);
void save_current_context(struct interrupt_regs *r);
void dq_decrement_head(struct embedded_link *);
struct task_struct *dq_dequeue(struct embedded_link *);
void dq_enqueue(struct embedded_link *, struct task_struct *);
#endif

#endif /* !_SCH_H */
