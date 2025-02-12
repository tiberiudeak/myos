#include <arch/i386/gdt.h>
#include <arch/i386/isr.h>
#include <arch/i386/pit.h>
#include <kernel/list.h>
#include <kernel/shell.h>
#include <kernel/string.h>
#include <kernel/tty.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <process/process.h>
#include <process/scheduler.h>

#include <stddef.h>

// ready task queue
struct embedded_link task_queue;
// sleeping task queue
struct embedded_link sleep_task_dqueue;
struct task_struct *current_running_task;
uint8_t scheduler_initialized = 0;

#ifdef CONFIG_RR_TIME_QUANTUM
const uint32_t running_time_quantum_ms = CONFIG_RR_TIME_QUANTUM;
#endif

/**
 * @brief Put task in task queue
 *
 * This function puts the given task in the task queue:
 *
 * @param task			The task to be put in the queue
 */
void enqueue_task(struct task_struct *task) {
	struct task_node *new_node;

	new_node = kmalloc(sizeof(struct task_node));

	if (new_node != NULL) {
		new_node->task = task;

		list_add_end(&task_queue, &new_node->list);
	}
}

/**
 * @brief Get task from the task queue
 *
 * This fuction takes out and returns a task from the task
 * queue if available.
 *
 * @return The task taken out from the queue
 */
struct task_struct *dequeue_task(void) {
	struct task_node *first_node;
	struct task_struct *task;

	if (list_is_empty(&task_queue)) {
		return NULL;
	}

	first_node = list_get_entry(task_queue.next, struct task_node, list);
	task = first_node->task;

	list_delete(&task_queue, task_queue.next);
	kfree(first_node);

	return task;
}

#ifdef CONFIG_FCFS_SCH
/**
 * @brief Initialize the scheduler task queue
 *
 * This function calls the init_task_queue() function
 * to initialize the task queue. Only used as a wrapper
 * for the other function.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t scheduler_init(void) {
	return init_task_queue();
}

/**
 * @brief Initialize the scheduler task queue
 *
 * This function performs the actual initialization of the task
 * queue.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t init_task_queue(void) {
	list_init(&task_queue);

	scheduler_initialized = 1;

	return 0;
}

/**
 * @brief Check task queue and execute tasks if available
 *
 * This function checks if there are available tasks in the task queue. If
 * present, the task is taken out from the queue and executed. If none are
 * present, then the scheduler will sleep for some time and then look again
 * for tasks in the queue.
 *
 */
void simple_task_scheduler(void) {
	while (1) {
		if (list_is_empty(&task_queue)) {
			// sleep a bit
			wait_millis(100);

			continue;
		}

		struct task_struct *task = dequeue_task();
		current_running_task = task;

		void (*program)(int argc, char *argv[]);
		program = (void (*)(int, char **)) task->exec_address;

		// change virtual address space
		if (task->vas != NULL) {
			set_page_directory(task->vas);
		}

		// execute task
		program(task->argc, task->argv);

		// following code is executed only if the program could not be executed
		// for some reason (file not present, not an ELF file, etc...)
		destroy_task(task);
		shell_cleanup();
	}
}
#else

/*
 * decrement delta time of first task in the sleeping delta queue
 *
 * @param delta_queue_h		head of delta queue
 */
void dq_decrement_head(struct embedded_link *delta_queue_h) {
	struct delta_queue_node *dqn;

	if (!list_is_empty(delta_queue_h)) {
		dqn = get_container(delta_queue_h->next, struct delta_queue_node, list);

		dqn->delta_time_ms--;
	}
}

/*
 * get element from delta queue
 *
 * @param delta_queue_h		head of delta queue
 */
struct task_struct *dq_dequeue(struct embedded_link *delta_queue_h) {
	if (list_is_empty(delta_queue_h)) {
		return NULL;
	}

	struct delta_queue_node *dqn =
		get_container(delta_queue_h->next, struct delta_queue_node, list);
	struct task_struct *ts = dqn->task;
	list_delete(delta_queue_h, delta_queue_h->next);
	kfree(dqn);

	return ts;
}

/*
 * add task struct to the delta queue
 *
 * @param delta_queue_h		head of delta queue
 * @param ts				task struct to add
 */
void dq_enqueue(struct embedded_link *delta_queue_h, struct task_struct *ts) {
	struct delta_queue_node *new_dqn = kmalloc(sizeof(struct delta_queue_node));
	struct delta_queue_node *dqn;
	struct embedded_link *cursor;
	int wakeup_time = ts->sleep_time;

	if (!new_dqn) {
		return;
	}

	new_dqn->task = ts;
	new_dqn->delta_time_ms = ts->sleep_time;

	if (list_is_empty(delta_queue_h)) {
		list_add_front(delta_queue_h, &new_dqn->list);
		return;
	}

	list_iterate(cursor, delta_queue_h) {
		dqn = list_get_entry(cursor, struct delta_queue_node, list);

		if (dqn->delta_time_ms < wakeup_time) {
			wakeup_time -= dqn->delta_time_ms;
		} else {
			// add in the middle or at the beginning
			new_dqn->delta_time_ms = wakeup_time;
			list_add_before(delta_queue_h, cursor, &new_dqn->list);
			dqn->delta_time_ms -= wakeup_time;
			return;
		}
	}

	// add at the end
	new_dqn->delta_time_ms = wakeup_time;
	list_add_end(delta_queue_h, &new_dqn->list);
}

/**
 * @brief Get size of the specified queue
 *
 * This function returns the size of the specified queue.
 *
 * @param queue_type	Queue type
 * @return The queue size
 */
uint32_t queue_size(struct embedded_link *list_h) {
	uint32_t count = 0;
	struct embedded_link *cursor;

	list_iterate(cursor, list_h) {
		count++;
	}

	return count;
}

/**
 * @ Init task
 *
 * This function represents the first task created and executed.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 */
void init_task_func(int argc, char *argv[]) {
	(void) argv; // remove compilation warning
	(void) argc;

	// change stack to init task kstack
	// -- done here, as the init task is not started
	// by the scheduler (but maybe it would be a good idea
	// to start it from pit like any other task -- TODO)
	__asm__ __volatile__("cli; mov %%eax, %%esp; mov %%esp, %%ebp; sti" ::"a"(
		current_running_task->kstack));

	scheduler_initialized = 1;

#ifdef CONFIG_VERBOSE
	printk("init process started!\n");
#endif
	while (1) {
		__asm__ __volatile__("sti; hlt; cli");
	}
}

/**
 * @brief Initialize the round-robin scheduler's queues
 *
 * This function performs the actual initialization of the task
 * queues by creating the sleeping queue and creating and adding
 * the init task to the running queue
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t init_queues_rr(void) {
	// init task queue
	list_init(&task_queue);

	// init delta queue
	list_init(&sleep_task_dqueue);

	char **argv = kmalloc(sizeof(char *) * 1);

	if (argv == NULL) {
		goto argv_err;
	}

	argv[0] = kmalloc(sizeof(char) * 10);

	if (argv[0] == NULL) {
		goto argv0_err;
	}

	strcpy(argv[0], "init");

	// create init task and add it to the queue
	void (*init)(int, char **) = init_task_func;
	struct task_struct *init_task = create_task(init, 1, argv, 0);

	if (init_task == NULL) {
		printk("init task is NULL!\n");
		goto null_init_task_err;
	}

	enqueue_task(init_task);

	kfree(argv[0]);
	kfree(argv);

	return 0;

null_init_task_err:
	kfree(argv[0]);
argv0_err:
	kfree(argv);
argv_err:
	return 1;
}

/**
 * @brief Start init task
 *
 * This function takes a task from the task queue (when this function
 * is called, the only task in the task queue will be init), updates
 * the current running task and starts its execution.
 */
void start_init_task(void) {
	// take init task from the queue
	struct task_struct *task = dequeue_task();

	// update current running task
	current_running_task = task;

	// start execution of init
	void (*program)(int argc, char *argv[]);
	program = (void (*)(int, char **)) task->exec_address;
	program(task->argc, task->argv);
}

/**
 * @brief Initialize the round-robin scheduler
 *
 * This function calls the init_task_queue_rr() function
 * to initialize the queues. Only used as a wrapper
 * for the other function.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t scheduler_init_rr(void) {
	return init_queues_rr();
}

/**
 * @brief Change contex for a user space task in the TASK_CREATED
 *        state (described only by an instruction pointer and stack)
 *
 * -- only called for user space tasks without prior context --
 *
 * This function changes the context for a user space task
 * described only by its instruction pointer and stack.
 *
 * @param r		"Context" of the previous task that needs to
 *				be updated
 * @param prob	potential problem that needs special handling
 *				(see header file for more info)
 */
void change_context(struct interrupt_regs *r, TASK_SWITCH_STACK_PROBLEM prob) {
	// change DS and SS to user mode data segment selector
	*(&r->ds) = USER_DS;

	if (prob == NO_PROBLEM && current_running_task->ring == 3) {
		*(&r->ss) = current_running_task->context->ss;
		*(&r->useresp) = current_running_task->context->useresp;
	}

	// TODO: clear flags (except for the interrupt bit)

	// change CS to user mode code segment selector
	*(&r->cs) = USER_CS;

	// change instruction pointer
	*(&r->eip) = current_running_task->context->eip;

	/*
	 * TODO: update TSS for ring 0
	 *
	 * right now, all tasks running in user space will use the kernel
	 * stack defined in TSS when entering the kernel (through syscalls,
	 * hardware interrupts, etc.), which is the same for all. Every kernel
	 * task however will have its own kernel stack to avoi stack corruption.
	 * For now, using the same stack for user space processes is alright, as
	 * they cannot be interrupted while executing kernel code, so the stack
	 * will maintain its state. This will change in the future, and the TSS
	 * will have to be updated with the kernel stack of each user space task.
	 */
}

/*
 * @brief Save context of current running task
 *
 * When a task is interrupted, its context is pushed on the stack by the
 * interrupt handler. This function gets the context and saves it in the
 * task's task_struct, later used to resume its execution.
 *
 * @param r		The context saved by the interrupt handler on the stack
 */
void save_current_context(struct interrupt_regs *r) {
	current_running_task->context->flags = r->eflags;
	current_running_task->context->cs = r->cs;
	current_running_task->context->eip = r->eip;
	current_running_task->context->ebp = r->ebp;

	/* get the value of esp before the CPU has pushed any registers
	   on the stack (like EIP, CS, EFLAGS, SS, USERESP) */
	if (current_running_task->ring == 0) {
		/* add 0x14 to esp because when esp is stored,
		   there are already EFLAGS, CS, EIP, int_no and err_code
		   on the stack (and we want to skip them) */
		current_running_task->context->esp = r->esp + 0x14;
	} else if (current_running_task->ring == 3) {
		/* add 0x20 to esp because when esp is stored,
		   there are already SS, ESP, EFLAGS, CS, EIP, int_no
		   and err_code on the stack (and we want to skip them) */
		current_running_task->context->esp = r->esp + 0x20;
	}

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
	current_running_task->context->ss = r->ss;
	current_running_task->context->useresp = r->useresp;
}

/**
 * @brief Change contex for a kernel space task in the TASK_CREATED state
 *
 * -- only called for kernel space tasks without prior context --
 *
 * This function changes the context for a kernel space task. The task
 * is described only by its EIP and ESP in its task_struct. The stack
 * will be changed later by the interrupt handler (see CHANGE_KSTACK
 * in the header file)
 *
 * @param r		"Context" of the previous task that needs to
 *				be updated
 */
void change_context_kernel(struct interrupt_regs *r) {
	*(&r->ds) = KERNEL_DS;
	*(&r->cs) = KERNEL_CS;
	*(&r->eip) = current_running_task->context->eip;
}

/**
 * @brief Switch context
 *
 * This functions switches the context of the previous running
 * task with the context of another task (that has become the
 * current running task).
 *
 * @param r		"Context" of the previous running task that needs to
 *				be updated
 * @param prob	potential problem that needs special handling
 *				(see header file for more info)
 */
void resume_context(struct interrupt_regs *r, TASK_SWITCH_STACK_PROBLEM prob) {
	// restore segments
	*(&r->ds) = current_running_task->context->ds;
	*(&r->cs) = current_running_task->context->cs;

	// restore general registers
	*(&r->eax) = current_running_task->context->eax;
	*(&r->ebx) = current_running_task->context->ebx;
	*(&r->ecx) = current_running_task->context->ecx;
	*(&r->edx) = current_running_task->context->edx;
	*(&r->edi) = current_running_task->context->edi;
	*(&r->esi) = current_running_task->context->esi;

	// restore flags
	*(&r->eflags) = current_running_task->context->flags;

	// restore kernel stack
	*(&r->ebp) = current_running_task->context->ebp;
	*(&r->esp) = current_running_task->context->esp;

	// restore instruction pointer
	*(&r->eip) = current_running_task->context->eip;

	// restore user space stack info only for user space tasks
	if (prob == NO_PROBLEM && current_running_task->ring == 3) {
		*(&r->ss) = current_running_task->context->ss;
		*(&r->useresp) = current_running_task->context->useresp;
	}
}

/**
 * @brief Round-Robin Scheduler
 *
 * This function puts the current running task in the specified queue
 * (if the task is not terminated), takes a task from the running queue
 * and updates the current running task with the new task. The function
 * also changes the virtual address space if the new task runs in user space.
 *
 * @param queue_type	Queue where to put the current_running_task
 */
void schedule(QUEUE_TYPE queue_type) {
	// put current running task in the queue if task
	// if not terminated
	if (current_running_task != NULL) {
		if (queue_type == RUNNING_TASK_QUEUE) {
			enqueue_task(current_running_task);
		} else {
			dq_enqueue(&sleep_task_dqueue, current_running_task);
		}
	}

	// take task from the running queue and update current running task
	struct task_struct *task = dequeue_task();
	current_running_task = task;

	// change virtual address space for user tasks
	if (task->vas != NULL) {
		set_page_directory(task->vas);
	} else {
		set_kernel_page_directory();
	}
}

/*
 * display processes in the task queue - called for ps command
 */
void display_running_processes(void) {
	struct embedded_link *cursor;
	struct delta_queue_node *dqn;
	struct task_node *tn;

	printk("id: %d %s (running)\n", current_running_task->task_id,
		   current_running_task->argv[0]);

	list_iterate(cursor, &task_queue) {
		tn = list_get_entry(cursor, struct task_node, list);

		printk("id: %d %s\n", tn->task->task_id, tn->task->argv[0]);
	}

	list_iterate(cursor, &sleep_task_dqueue) {
		dqn = list_get_entry(cursor, struct delta_queue_node, list);

		printk("id: %d %s (blocked for %dms)\n", dqn->task->task_id,
			   dqn->task->argv[0], dqn->delta_time_ms);
	}
}

#endif
