#include <arch/i386/gdt.h>
#include <kernel/shell.h>
#include <kernel/string.h>
#include <kernel/tty.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <process/process.h>
#include <process/scheduler.h>

static uint32_t next_available_task_id = 1;
extern struct task_struct *current_running_task;

/**
 * @brief Create a task
 *
 * This function created a new task based on the given parameters.
 *
 * @param exec_address The instruction pointer to the beginning of the task
 * (only used for the simple scheduler)
 * @param argc         The arguments conunt
 * @param argv         The values for the arguments
 * @userspace          Set to 1 if task will run in userspace, 0 otherwise
 *
 * @return The created task
 */
struct task_struct *create_task(void *exec_address, int argc, char **argv,
								int userspace) {
	struct task_struct *task =
		(struct task_struct *) kmalloc(sizeof(struct task_struct));
	void *k_stack = NULL;
#ifdef CONFIG_VERBOSE
	printk("create task: %s\n", argv[0]);
#endif

	if (!task) {
		goto task_err;
	}

	task->state = TASK_CREATED;
	task->task_id = next_available_task_id++;
	task->exec_address = exec_address;
	task->argc = argc;
	task->run_time = 0;
	task->sleep_time = 0;
	task->maps = NULL;

	task->context = kmalloc(sizeof(struct proc_context));

	if (!task->context) {
		goto task_context_err;
	}

	if (userspace) {
		task->vas = create_address_space();

		if (!task->vas) {
			goto task_vas_err;
		}

		task->ring = 3;
		task->context->cs = USER_CS;
		task->context->ds = USER_DS;
		task->context->es = USER_DS;
		task->context->fs = USER_DS;
		task->context->gs = USER_DS;
		task->context->ss = USER_DS;
	} else {
		task->vas = NULL;
		task->ring = 0;
		task->context->eip = (uint32_t) exec_address;
		task->context->cs = KERNEL_CS;
		task->context->ds = KERNEL_DS;
		task->context->es = KERNEL_DS;
		task->context->fs = KERNEL_DS;
		task->context->gs = KERNEL_DS;
		task->context->ss = KERNEL_DS;

		k_stack = kmalloc(sizeof(char) * KSTACK_SIZE);

		if (!k_stack) {
			goto task_kstack_err;
		}

		task->kstack = k_stack + KSTACK_SIZE;
	}

	if (argv != NULL) {
		task->argv = (char **) kmalloc(sizeof(char *) * argc);

		if (!task->argv) {
			goto task_argv_err;
		}
	}

	for (int i = 0; i < argc; i++) {
		task->argv[i] = (char *) kmalloc(sizeof(char) *
										 MAX_PARAM_SIZE); // max 10 characters

		if (!task->argv[i]) {
			// free previous allocated argvs
			for (int j = 0; j < i; j++) {
				kfree(task->argv[j]);
			}

			goto task_argvs_err;
		}

		strcpy(task->argv[i], argv[i]);
	}

	return task;

task_argvs_err:
	kfree(task->argv);
task_argv_err:
	if (k_stack != NULL) {
		kfree(k_stack);
	}
task_vas_err:
task_kstack_err:
	kfree(task->context);
task_context_err:
	kfree(task);
task_err:
	return NULL;
}

/**
 * @brief Free memory for the given task from the kernel heap
 *
 * This function frees all memory allocated on the kernel heap used
 * for the given task.
 *
 * @param task The task
 */
void destroy_task(struct task_struct *task) {
	// free memory used for maps
	struct mapping *tmp = task->maps, *tmp2;

	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = (struct mapping *) tmp->next;
		kfree(tmp2);
	}

	for (int i = 0; i < task->argc; i++) {
		kfree(task->argv[i]);
	}

	kfree(task->argv);
	kfree(task->context);
	kfree(task);
}

// function called at the end of a kernel task
// -- it is mandatory to be called by every kernel task
// that terminates its execution
void ktask_exit(void) {
	/*
	 * mark current task as terminated
	 * and wait for scheduler to schedule another task
	 *
	 * cannot schedule task now as no IRET instruction is
	 * called, and thus no task can run after this
	 */
	current_running_task->state = TASK_TERMINATED;

	while (1) {
		__asm__ __volatile__("sti; hlt; cli");
	}
}

#ifdef CONFIG_FCFS_SCH
void enter_usermode(uint32_t entry_point, uint32_t stack_address) {
	// save current context somehow to restore it when exit is called?
	// maybe update the TSS
	//__asm__ __volatile__ ("mov %%esp, %%edx" : "=d"(kernel_context.esp));
	//__asm__ __volatile__ ("mov %%ebp, %%edx" : "=d"(kernel_context.ebp));
	//__asm__ __volatile__ ("pushf\n" "pop %%edx" :
	//"=d"(kernel_context.eflags));

	__asm__ __volatile__(
		"cli\n"				 // critical section, cannot be interrupted
		"mov $0x23, %%eax\n" // set segments to user mode data segment selector
		"mov %%eax, %%ds\n"
		"mov %%eax, %%es\n"
		"mov %%eax, %%fs\n"
		"mov %%eax, %%gs\n"

		"push $0x23\n"		 // 0x23 is the user mode data segment selector
		"push %%ebx\n"		 // push the user stack address
		"pushf\n"			 // push eflags register
		"pop %%eax\n"		 // pop the flags registers into EAX
		"or $0x200, %%eax\n" // enable the interrupt bit in the flags register
		"push %%eax\n"		 // put back the flags on the stack
		"push $0x1b\n"		 // 0x1b is the user mode code segment selector
		"push %1\n"			 // push the instruction pointer

		"iret\n" // perform iret
		:
		: "b"(stack_address), "r"(entry_point));
}
#endif /* CONFIG_FCFS_SCH */
