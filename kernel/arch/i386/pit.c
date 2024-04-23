#include "include/process/process.h"
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

// data from the scheduler
extern const uint32_t running_time_quantum_ms;
extern task_queue_t *task_queue;
extern uint8_t scheduler_initialized;
extern task_struct *current_running_task;

/**
 * @brief Increment ticks
 *
 * This is the IRQ0 handler that increments the ticks every
 * one millisecond and the uptime.
 */
void PIT_IRQ0_handler(interrupt_regs *r) {
    uint8_t ret = 0;
	ticks++;
	uptime++;


    // only for the round robin scheduler
    // if there is at least one task in the queue, this means that a new task was
    // added in the queue (the init task is currently executing, so the queue is empty)
    if (ticks % running_time_quantum_ms == 0 && scheduler_initialized && queue_size() != 0) {

        if (current_running_task->state != TASK_TERMINATED) {
            // save current running task context
            current_running_task->context->flags = r->eflags;
            current_running_task->context->cs = r->cs;
            current_running_task->context->eip = r->eip;
            current_running_task->context->ebp = r->ebp;
            current_running_task->context->esp = r->useresp;
            current_running_task->context->edi = r->edi;
            current_running_task->context->esi = r->esi;
            current_running_task->context->edx = r->edx;
            current_running_task->context->ecx = r->ecx;
            current_running_task->context->ebx = r->ebx;
            current_running_task->context->eax = r->eax;
            current_running_task->context->ds = r->ds;
            current_running_task->context->es = r->ds;
            current_running_task->context->fs = r->ds;
            current_running_task->context->gs = r->ds;

            current_running_task->state = TASK_READY;
        }

        if (current_running_task->state == TASK_TERMINATED) {
            destroy_task(current_running_task);
            current_running_task = NULL;
        }
sch:
        // call scheduler
        schedule();

        // prepare elf execution if user space process
        if (current_running_task->vas != NULL && current_running_task->state == TASK_CREATED) {
            ret = prepare_elf_execution(current_running_task->argc, current_running_task->argv);

            if (ret) {
                current_running_task = NULL;
                goto sch;
            }
        }

        // update registers on the stack for the new task
        if (current_running_task->state == TASK_CREATED) {
            change_context(r);
            current_running_task->state = TASK_RUNNING;
        }
        else {
            resume_context(r);
        }
    }
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

    void (*timer_handler)(interrupt_regs *r) = PIT_IRQ0_handler;
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
	ticks = 0;

	while (ticks < millis) __asm__ __volatile__ ("sti; hlt; cli");
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
