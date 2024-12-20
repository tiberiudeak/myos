#include "include/process/scheduler.h"
#include <arch/i386/isr.h>
#include <arch/i386/pit.h>
#include <arch/i386/irq.h>
#include <kernel/io.h>
#include <kernel/tty.h>
#include <process/process.h>
#include <process/scheduler.h>
#include <mm/kmalloc.h>
#include <elf.h>

uint32_t ticks;
uint32_t uptime;
uint32_t wait_ticks;

// data from the scheduler
extern const uint32_t running_time_quantum_ms;
extern struct task_queue *task_queue;
extern struct task_queue *sleep_task_queue;
extern uint8_t scheduler_initialized;
extern struct task_struct *current_running_task;

/**
 * @brief Timer interrupt handler
 *
 * This is the IRQ0 handler that increments the ticks every
 * one millisecond and the uptime.
 *
 * This is also the interrupt used by the round-robin scheduler
 * to switch between tasks.
 */
void PIT_IRQ0_handler(struct interrupt_regs *r) {
	struct task_node *tn;
	struct task_struct *ts;
    uint8_t ret = 0;
	ticks++;
	uptime++;
	wait_ticks++;

#ifndef CONFIG_FCFS_SCH
	current_running_task->run_time++;

	// update blocked tasks (from the sleeping queue)
	if (scheduler_initialized && queue_size(SLEEPING_TASK_QUEUE) != 0) {
		tn = sleep_task_queue->front;

		while (tn != NULL) {
			tn->task->sleep_time--;

			if (--tn->task->sleep_time == 0) {
				ts = dequeue_task(SLEEPING_TASK_QUEUE);
				enqueue_task(ts, RUNNING_TASK_QUEUE);
			}

			tn = tn->next;
		}
	}

    // only for the round robin scheduler
    // if there is at least one task in the queue, this means that a new task was
    // added in the queue (the init task is currently executing, so the queue is empty)
    if (current_running_task->run_time % running_time_quantum_ms == 0 && scheduler_initialized &&
			queue_size(RUNNING_TASK_QUEUE) != 0) {

        // save current running task's context
        if (current_running_task->state != TASK_TERMINATED) {
			save_current_context(r);

            // change task state from RUNNING to READY
            current_running_task->state = TASK_READY;

			// reset run time
			current_running_task->run_time = 0;
        }

        // if task finished execution, free memory
        if (current_running_task->state == TASK_TERMINATED) {
            destroy_task(current_running_task);
            current_running_task = NULL;
        }

rr_sch:
        // call scheduler
        schedule(RUNNING_TASK_QUEUE);

        // prepare elf execution if user space process
        if (current_running_task->vas != NULL && current_running_task->state == TASK_CREATED) {
            ret = prepare_elf_execution(current_running_task->argc, current_running_task->argv);

            if (ret) {
                // failed to prepare elf file, terminate task and call scheduler again
                current_running_task = NULL;
                current_running_task->state = TASK_TERMINATED;
                goto rr_sch;
            }
        }

        // update registers on the stack (context) for the new task
        if (current_running_task->state == TASK_CREATED) {
            change_context(r);
        }
        else {
            resume_context(r);
        }

        current_running_task->state = TASK_RUNNING;
    }
#endif /* !CONFIG_FCFS_SCH */
}

uint32_t random(void) {
    return ((uptime * 214013L + 2531011L) >> 16) & 0x7FFF;
}

/**
 * @brief Initialize the Programmable Interrupt Timer
 *
 * This function first calculates the appropriate divisor to achieve a delay
 * of one millisecond, then configures the PIT and install the handler for IRQ0.
 */
void PIT_init() {

	uint32_t divisor = PIT_FREQ / 1000;

	port_byte_out(COMMAND_PORT, PIT_CW_RL_LSB_MSB | PIT_CW_MODE3);

	// send divisor
	port_byte_out(CH0_DATA_PORT, divisor & 0xFF);
	port_byte_out(CH0_DATA_PORT, (divisor >> 8) & 0xFF);


	ticks = 0;
	uptime = 0;

    void (*timer_handler)(struct interrupt_regs *r) = PIT_IRQ0_handler;
	irq_install_handler(0, timer_handler);
}

/**
 * @brief Wait the given milliseconds
 *
 * This function waits for the given milliseconds to pass. The global variable
 * ticks is incremented every millisecond.
 *
 * @param millis The number of milliseconds to wait
 */
void wait_millis(uint16_t millis) {
	wait_ticks = 0;

	while (wait_ticks < millis) __asm__ __volatile__ ("sti; hlt; cli");
}

/**
 * @brief Get uptime since boot in milliseconds
 *
 * This function returns roughly the number of milliseconds tat passed
 * since boot
 */
uint64_t get_uptime() {
	return uptime;
}
