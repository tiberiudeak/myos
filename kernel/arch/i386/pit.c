#include <arch/i386/isr.h>
#include <arch/i386/pit.h>
#include <arch/i386/irq.h>
#include <kernel/io.h>
#include <kernel/list.h>
#include <kernel/tty.h>
#include <process/process.h>
#include <process/scheduler.h>
#include <mm/kmalloc.h>
#include <elf.h>

uint32_t ticks;
uint32_t uptime;
uint32_t wait_ticks;
TASK_SWITCH_STACK_PROBLEM irq_prob;

// data from the scheduler
extern const uint32_t running_time_quantum_ms;
extern struct embedded_link task_queue;
extern struct embedded_link sleep_task_dqueue;
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
	struct delta_queue_node *dqn;
	struct task_struct *ts;
    uint8_t ret = 0;
	int prev_ring;

	ticks++;
	uptime++;
	wait_ticks++;

#ifndef CONFIG_FCFS_SCH
	current_running_task->run_time++;
	prev_ring = current_running_task->ring;

	// update blocked tasks (from the sleeping queue)
	if (scheduler_initialized && !list_is_empty(&sleep_task_dqueue)) {
		dq_decrement_head(&sleep_task_dqueue);

		dqn = list_get_entry(sleep_task_dqueue.next, struct delta_queue_node, list);

		if (dqn->delta_time_ms <= 0) {
			ts = dq_dequeue(&sleep_task_dqueue);
			enqueue_task(ts);
		}
	}

    // if there is at least one task in the queue, this means that a new task was
    // added in the queue (the init task is currently executing, so the queue is empty)
    if (current_running_task->run_time >= running_time_quantum_ms && scheduler_initialized &&
			!list_is_empty(&task_queue)) {

        // save current running task's context
        if (current_running_task->state != TASK_TERMINATED) {
			save_current_context(r);

            // change task state from RUNNING to READY
            current_running_task->state = TASK_READY;

			// reset run time
			current_running_task->run_time = 0;
        } else {
			// if task finished execution, free memory
            destroy_task(current_running_task);
            current_running_task = NULL;
		}

rr_sch:
        // call scheduler
        schedule(RUNNING_TASK_QUEUE);

		if (prev_ring == 0 && current_running_task->ring == 3) {
			irq_prob = MANUAL_PUSH;
		} else if (prev_ring == 3 && current_running_task->ring == 0) {
			irq_prob = MANUAL_POP;
		}

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
			if (current_running_task->ring == 3) {
				change_context(r, irq_prob);
			} else if (current_running_task->ring == 0) {
				change_context_kernel(r);
				irq_prob |= CHANGE_KSTACK;
			}
        }
        else {
            resume_context(r, irq_prob);
			if (current_running_task->ring == 0) {
				irq_prob |= RESUME_KSTACK;
			}
        }

        current_running_task->state = TASK_RUNNING;
    }
#endif /* !CONFIG_FCFS_SCH */
}

/*
 * @brief Generate "random" number
 *
 * @return The generated number
 */
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
