#ifndef MY_NCURSES_MENU_H
#define MY_NCURSES_MENU_H 1

#include <ncurses.h>

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

// lower text for navigation information - need to be increased if
// lines are added to the navigation_info string
#define MAIN_MENU_Y     6
#define MAIN_MENU_X     2

#define SEC_MENU_X      2
#define SEC_MENU_Y      2

typedef enum {
    INT, BOOL
} type;

struct Config {
    char *symbol;
    char *prompt;
    char *help_message;
    int default_val;
    type type;
    char *deps;
};

struct Choice {
    char *prompt;
    char *help_message;
    struct Config *configs;
    int n_configs;
};

struct Menu {
    char *prompt;
    char *help;
    struct Choice *choices;
    struct Config *configs;
    int n_choices;
    int n_configs;
};

// ==============================General Setup configurations=====================================
struct Config general_setup_configs[] = {
    {
        "CONFIG_TTY_VBE_WIDTH",
        "Graphics Mode Width",
        "Screen Width\n\n\
        This configuration allows you to set the horizontal resolution of your display in pixels.\n\
        You can choose from the following available resolutions (first column is the width):\n\n\
         320 x 200\n\
         640 x 400\n\
         640 x 480\n\
         800 x 600\n\
        1024 x 768\n\
        1280 x 1024\n\
        1600 x 1200\n\
        1152 x 864\n\
        1280 x 768\n\
        1280 x 800\n\
        1280 x 960\n\
        1440 x 900\n\
        1400 x 1050\n\
        1680 x 1050\n\
        1920 x 1200\n\
        2560 x 1600\n\
        1280 x 720\n\
        1920 x 1080\n\n\n\
        This configuration is only available in Graphics Mode!",
        1920,
        INT,
        "CONFIG_TTY_VBE"
    },

    {
        "CONFIG_TTY_VBE_HEIGHT",
        "Graphics Mode Height",
        "Screen Height\n\n\
        This configuration allows you to set the vertical resolution of your display in pixels.\n\
        You can choose from the following available resolutions (second column is the height):\n\n\
         320 x 200\n\
         640 x 400\n\
         640 x 480\n\
         800 x 600\n\
        1024 x 768\n\
        1280 x 1024\n\
        1600 x 1200\n\
        1152 x 864\n\
        1280 x 768\n\
        1280 x 800\n\
        1280 x 960\n\
        1440 x 900\n\
        1400 x 1050\n\
        1680 x 1050\n\
        1920 x 1200\n\
        2560 x 1600\n\
        1280 x 720\n\
        1920 x 1080\n\n\n\
        This configuration is only available in Graphics Mode!",
        1080,
        INT,
        "CONFIG_TTY_VBE"
    },

    {
        "CONFIG_VERBOSE",
        "Verbose",
        "Verbose Configuration\n\n\
        This configuration enables the display of detailed kernel messages during the boot\n\
        process and operation of the operating system. When enabled, the kernel will output\n\
        a set of messages that provide insights into the system's activities, initialization\n\
        sequences and potential issues.",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_RTC",
        "Real time clock",
        "Real Time Clock\n\n\
        This configuration enables the Real-Time Clock (RTC) functionality in the operating\n\
        system. When enabled, the system can maintain and provide accurate timekeeping, which\n\
        is crucial for various time-dependent process and functionalities. Additionally, it\n\
        introduces the 'datetime' command, offering convenient way for users to access the\n\
        current date and time from the command line.",
        1,
        BOOL,
        NULL
    }
#ifdef STEP_BY_STEP
    ,{
        "CONFIG_DONE",
        "Done",
        "Select when done with the configurations in the current menu in order to have access\n\
        to the next menu",
        0,
        BOOL,
        NULL
    }
#endif
};

struct Config video_mode_configs[] = {
    {
        "CONFIG_TTY_VGA",
        "VGA Text Mode",
        "VGA Text Mode\n\n\
        This mode uses the classic VGA (Video Graphics Aray) text mode, which provides\n\
        a simple and efficient way to display text on the screen. It is characterized\n\
        by its fixed 80x25 resolution, meaning the screen is divided into 80 columns\n\
        and 25 rows of characters.\n\n\
        Features:\n\
        - Resolution: 80x25\n\
        - Colors: supports 16 colors\n\
        - Performance: low memory and CPU usage, making it ideal for systems with limited\n\
                        resources or for debugging purposes\n\
        - Compatibility: highly compatible with older hardware and software that relies\n\
                        on text based interfaces\n\n\
        Use Cases:\n\
        - Text-based applications\n\
        - Command-line interfaces\n\
        - Low-resource environments\n\
        - Debugging and recovery modes",
        1,
        BOOL,
        NULL},

    {
        "CONFIG_TTY_VBE",
        "Graphics Mode",
        "Graphics Mode (VESA BIOS Extension)\n\n\
        This mode leverages the VESA (Video Electronics Standards Association) BIOS\n\
        Extension to provide higher resolutions and color depth for graphical interfaces.\n\n\
        Features:\n\
        - Resolution: supports a wide range of resolutions:\n\
        - Color Depth: 32-bit colors, allowing for millions of colors\n\
        - Performance: requires more memory and CPU resources compared to the text mode,\n\
                        but provides a richer and more flexible visual experience\n\
        - Compatibility: requires hardware that supports VBE, which is standard in most\n\
                        modern graphics cards\n\n\
        Use Cases:\n\
        - Graphical user interfaces\n\
        - Any scenario requiring high-resolution graphics and extensive color palettes",
        0,
        BOOL,
        NULL
    }
};

struct Choice general_setup_choices[] = {
    {
        "Video Mode",
        "Video Mode Configuration\n\n\
        The video mode confguration allows you to select the display mode for the operating\n\
        system's graphical output. This setting determines how information is presented on\n\
        the screen.",
        video_mode_configs,
        ARRAY_SIZE(video_mode_configs)
    }
};
// ===============================================================================================



// ==============================Memory Manager configurations====================================
struct Config memory_manager_configs[] = {
    {
        "CONFIG_READ_AFTER_FREE_PROT",
        "Read after free protection",
        "Read After Free Protection\n\n\
        Read After Free Protection provides an additional layer of security and stability for\n\
        memory management within the operating system. When this feature is enabled, the memory\n\
        blocks that are freed by the system are overwritten with zeros. This practice helps to\n\
        prevent potential security vulnerabilities and bugs that can arise from accessing stale\n\
        data.\n\n\
        Performance Considerations:\n\n\
        Enabling this protection introduces a slight performance overhead due to the additional\n\
        step of zeroing out memory blocks. However, the trade-off is generally favorable given the\n\
        increased security and stability.",
        1,
        BOOL,
        NULL
    }
#ifdef STEP_BY_STEP
    ,{
        "CONFIG_DONE",
        "Done",
        "Select when done with the configurations in the current menu in order to have access\n\
        to the next menu",
        0,
        BOOL,
        NULL
    }
#endif
};

struct Config mem_alloc_alg_configs[] = {
    {
        "CONFIG_UVMM_BESTFIT",
        "Best Fit",
        "Best Fit Memory Allocation Algorithm\n\n\
        Best Fit Memory Allocation is a memory management technique that allocates the smallest\n\
        free partition that is adequate to accommodate the requested memory. This approach aims\n\
        to minimize wasted memory by fitting the allocation as well as possible into the\n\
        available free space.\n\n\
        How it works:\n\n\
        When a memory allocation request is made, the memory manager searches the list of available\n\
        free memory blocks. It selects the smallest block that is large enough to fulfill the\n\
        request. The chosen block is then split if necessary, with the remaining part staying\n\
        available for future allocations.\n\n\
        Pros:\n\n\
        - Minimizes Wasted Space: By choosing the smallest possible block that fits the request,\n\
        Best Fit aims to reduce fragmentation and make better use of available memory.\n\
        - Efficient Use of Memory: Generally results in more efficient use of memory over time\n\
        compared to some other allocation strategies.\n\n\
        Cons:\n\n\
        - Slower Allocation: The search for the smallest fitting block can be time-consuming,\n\
        leading to slower allocation times compared to simpler strategies like First Fit.\n\n\
        Use Cases:\n\n\
        - Memory-Constrained Environments: Ideal for systems with limited memory resources where\n\
        maximizing memory usage is crucial.\n\
        - Applications with Variable Memory Requests: Useful in applications where the size of\n\
        memory requests varies significantly and efficient memory usage is more critical than the\n\
        speed of allocation.",
        1,
        BOOL,
        NULL
    },

    {
        "CONFIG_UVMM_FIRSTFIT",
        "First Fit",
        "First Fit Memory Allocation Algorithm\n\n\
        First Fit Memory Allocation is a memory management technique that allocates the first\n\
        free partition that is large enough to accommodate the requested memory. This approach\n\
        aims to minimize allocation time by quickly finding a suitable block.\n\n\
        How it works:\n\n\
        When a memory allocation request is made, the memory manager searches the list of available\n\
        free memory blocks from the beginning. It selects the first block that is large enough to\n\
        fulfill the request. The chosen block is then split if necessary, with the remaining part\n\
        staying available for future allocations.\n\n\
        Pros:\n\n\
        - Fast Allocation: The allocation process is generally faster because the search stops as\n\
        soon as a suitable block is found.\n\
        - Simple Implementation: The algorithm is straightforward to implement and requires less\n\
        computational overhead compared to more complex strategies.\n\n\
        Cons:\n\n\
        - Potential for Fragmentation: Over time, First Fit can lead to fragmentation as free\n\
        blocks are not used as efficiently as in Best Fit or other algorithms.\n\
        - Inefficient Memory Use: Larger blocks may be split early, leaving smaller fragments that\n\
        might be less useful for future allocations.\n\n\
        Use Cases:\n\n\
        - General-Purpose Systems: Suitable for systems where fast allocation times are more\n\
        critical than memory efficiency.\n\
        - Low-Latency Applications: Ideal for applications where quick response times are essential\n\
        and memory fragmentation is less of a concern.",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_UVMM_NEXTFIT",
        "Next Fit",
        "Next Fit Memory Allocation Algorithm\n\n\
        Next Fit Memory Allocation is a variant of the First Fit algorithm. In this approach,\n\
        the memory manager continues searching for a free block starting from the last allocated\n\
        block, rather than from the beginning of the memory each time.\n\n\
        How it works:\n\n\
        When a memory allocation request is made, the memory manager begins its search from the\n\
        location of the last allocated block. It selects the first block that is large enough to\n\
        fulfill the request. The chosen block is then split if necessary, with the remaining part\n\
        staying available for future allocations. The pointer or marker indicating the last\n\
        allocated block is updated to the location of the newly allocated block.\n\n\
        Pros:\n\n\
        - Reduced Allocation Time: Can be faster than First Fit in some cases because it avoids\n\
        repeatedly searching the same small blocks at the beginning of the memory.\n\
        - Simple Implementation: Like First Fit, it is relatively easy to implement and has low\n\
        computational overhead.\n\n\
        Cons:\n\
        - Potential for Fragmentation: Similar to First Fit, it can lead to fragmentation over\n\
        time.\n\
        - Inefficient Memory Use: Larger blocks might be split early, leaving smaller fragments\n\
        that may not be useful for future allocations.\n\
        - Variable Allocation Times: The time to find a suitable block can vary widely depending\n\
        on the location of the last allocation and the memory state.\n\n\
        Use Cases:\n\n\
        - Systems with Moderate Allocation Requests: Suitable for systems where allocations are\n\
        moderately frequent and the focus is on balancing speed and memory utilization.",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_UVMM_WORSTFIT",
        "Worst Fit",
        "Worst Fit Memory Allocation Algorithm\n\n\
        Worst Fit Memory Allocation is an approach that selects the largest available block of\n\
        memory for allocation.\n\n\
        How it works:\n\n\
        When a memory allocation request is made, the memory manager scans the entire list of\n\
        free memory blocks to find the largest block available. It selects this largest block to\n\
        fulfill the request. The chosen block is then split if necessary, with the remaining part\n\
        staying available for future allocations.\n\n\
        Pros:\n\n\
        Since this process selects the largest available partition, it results in significant\n\
        internal fragmentation. However, this leftover space is typically large enough to\n\
        accommodate other smaller processes.\n\n\
        Cons:\n\n\
        - Search Overhead: Scanning the entire list of free blocks to find the largest one can\n\
        be time-consuming, especially in systems with a large number of free blocks.\n\
        - Inefficient Use of Large Blocks: Can lead to inefficient use of large blocks if small\n\
        allocations are made, causing a large block to be broken into smaller, potentially unusable\n\
        fragments.\n\n\
        Use Cases:\n\n\
        Systems with Large Allocation Requests: Ideal for systems where large allocations are\n\
        frequent and minimizing fragmentation is critical.",
        0,
        BOOL,
        NULL
    }
};

struct Choice memory_manager_choices[] = {
    {
        "Memory Allocation Algorithm",
        "Memory Allocation Algorithms\n\n\
        The Memory Allocation Algorithms menu allows you to configure how the operating system\n\
        manages memory allocation for processes and applications. Memory allocation is a crucial\n\
        aspect of system performance and stability, and different algorithms can have significant\n\
        impacts on how efficiently and effectively the system uses available memory.\n\n\n\
        Considerations:\n\n\
        - Fragmentation: Some algorithms can lead to fragmented memory, where free memory is broken\n\
                         into small, non-contiguous blocks, making it difficult to allocate large\n\
                         blocks.\n\
        - Complexity vs Performance: More complex algorithms may provide better memory utilization\n\
                        but at the cost of increased overhead and slower allocation times.\n\
        - Application Requirements: Choose an algorithm that best fits the memory usage patterns of\n\
                        your applications to ensure efficient allocation and deallocation.\n\n\n\
        Usage Scenarios:\n\n\
        - High-Performance Systems: For systems requiring fast allocation and deallocation, choose\n\
                        algorithms prioritizing speed.\n\
        - Embedded Systems: For resource-constrained environments, select algorithms that optimize\n\
                        memory usage to ensure stability.\n\
        - General-Purpose Systems: Balance between complexity and performance to provide reliable\n\
                        and efficient memory management.",
        mem_alloc_alg_configs,
        ARRAY_SIZE(mem_alloc_alg_configs)}
};
// ===============================================================================================



// ==============================Scheduler configurations=========================================
struct Config scheduler_configs[] = {
    {
        "CONFIG_RR_TIME_QUANTUM",
        "Round-Robin Time Quantum",
        "Round Robin Time Quantum\n\n\
        The Round Robin Time Quantum configuration sets the length of time, in milliseconds,\n\
        that each process is allowed to run before the CPU switches to the next process in the\n\
        ready queue. This value is crucial in the Round Robin scheduling algorithm, determining\n\
        the maximum amount of time a process can execute before being preempted to give other\n\
        processes a chance to run.\n\n\
        Key Features:\n\n\
        - Time-Slice Allocation: Defines the duration each process can use the CPU before being\n\
                    preempted.\n\
        - Preemption: Ensures that processes are interrupted after their time quantum expires,\n\
                    allowing for a more equitable distribution of CPU time among processes.\n\
        - Responsiveness: Smaller time quantums can improve the system's responsiveness, making it\n\
                    suitable for interactive and real-time systems.\n\n\
        Considerations:\n\n\
        - Short Time Quantum:\n\
            Pros:\n\
                - Improved Responsiveness: Shorter time slices make the system more responsive,\n\
                    which is ideal for interactive applications where quick user feedback is\n\
                    important.\n\
                - Fairness: Ensures that all processes get frequent CPU access, improving overall\n\
                    fairness.\n\
            Cons:\n\
                - Increased Overhead: More frequent context switches can lead to higher overhead,\n\
                    reducing overall system efficiency.\n\
                - Higher Latency for Long Processes: Processes that require more CPU time may\n\
                    experience higher completion time due to frequent interruptions.\n\n\
        - Long Time Quantum:\n\
            Pros:\n\
                - Reduced Overhead: Fewer context switches decrease the overhead, potentially\n\
                    improving system efficiency.\n\
                - Better for CPU-Bound Processes: Longer time slices benefit CPU-bound processes,\n\
                    allowing them to complete tasks without frequent interruptions.\n\
            Cons:\n\
                - Reduced Responsiveness: Longer time slices can lead to slower response times,\n\
                    making the system less suitable for interactive applications.\n\
                - Potential for Uneven Distribution: Processes that require less CPU time might\n\
                    have to wait longer, reducing perceived fairness.\n\n\
        Choosing the optimal time quantum is a balance between responsiveness and efficiency.\n\
        The ideal value depends on the specific requirements of your system and the nature of the\n\
        processes running. Interactive Systems: Smaller time quantum (e.g., 20-50 milliseconds) to\n\
        ensure high responsiveness and quick user feedback. Batch Systems: Larger time quantum\n\
        (e.g., 100-200 milliseconds) to minimize overhead and allow longer processes to run more\n\
        efficiently.",
        20,
        INT,
        "CONFIG_ROUND_ROBIN"
    }
#ifdef STEP_BY_STEP
    ,{
        "CONFIG_DONE",
        "Done",
        "Select when done with the configurations in the current menu in order to have access\n\
        to the next menu",
        0,
        BOOL,
        NULL
    }
#endif
};

struct Config scheduler_alg_configs[] = {
    {
        "CONFIG_ROUND_ROBIN",
        "Round-Robin (RR)",
        "Round Robin Scheduling Algorithm\n\n\
        The Round Robin (RR) scheduling algorithm is a widely used and straightforward approach\n\
        for managing the execution of processes in a time-sharing operating system. In Round\n\
        Robin scheduling, each process is assigned a fixed time slice, or quantum, during which\n\
        it is allowed to execute. Once a process's quantum expires, the next process in the ready\n\
        queue is given the CPU. This cycle continues until all processes have completed their\n\
        execution.\n\n\
        Key Features:\n\n\
        - Time Slices (Quanta): Each process is assigned a small unit of CPU time called a time\n\
                        slice or quantum. The size of the quantum can significantly impact the\n\
                        performance and responsiveness of the system.\n\
        - Preemption: If a process does not finish its execution within its allocated time slice,\n\
                        it is preempted and moved to the back of the ready queue, allowing the next\n\
                        process to run.\n\
        - Fairness: RR ensures that all processes get an equal share of the CPU, preventing any\n\
                        single process from monopolizing the CPU.\n\n\
        Pros:\n\n\
        - Fairness: Each process gets an equal opportunity to execute, making RR ideal for\n\
                        time-sharing systems where multiple applications need to share the CPU.\n\
        - Responsiveness: RR provides good response times for short processes, as no process has to\n\
                        wait too long before getting another turn on the CPU.\n\n\
        Cons:\n\n\
        - Context Switching Overhead: Frequent context switches can occur, especially if the time\n\
                        quantum is small, leading to overhead that can degrade overall system\n\
                        performance.\n\
        - Performance Sensitivity: The performance of RR is highly sensitive to the choice of the\n\
                        time quantum. If the quantum is too large, RR behaves like FCFS; if too\n\
                        small, context switching overhead increases.\n\n\
        Use Cases:\n\n\
        - Time-Sharing Systems: RR is particularly well-suited for time-sharing systems where\n\
                        multiple users or tasks need to share the CPU fairly.\n\
        - Interactive Systems: Systems that require good responsiveness for user interactions,\n\
                        such as desktop operating systems and interactive applications, can\n\
                        benefit from RR scheduling.\n\n\n\
        By selecting the Round Robin scheduling algorithm, you ensure that your operating system\n\
        can handle multiple processes efficiently and fairly, providing a balanced approach to CPU\n\
        time allocation that can enhance the responsiveness and usability of the system.",
        1,
        BOOL,
        NULL
    },

    {
        "CONFIG_FCFS_SCH",
        "First Come First Served (FCFS)",
        "First Come First Served Scheduling Algorithm\n\n\
        The First Come First Served (FCFS) scheduling algorithm is the simplest and most\n\
        straightforward process scheduling approach used in this operating system. In FCFS\n\
        scheduling, processes are executed in the order they arrive in the ready queue. The\n\
        process that arrives first is executed first, and once a process starts execution, it\n\
        runs to completion without being preempted.\n\n\
        Key Features:\n\n\
        - Order of Arrival: Processes are executed in the exact order they arrive in the ready\n\
                queue.\n\
        - Non-Preemptive: Once a process starts executing, it runs until it finishes, without\n\
                being interrupted by other processes.\n\n\
        Pros:\n\n\
        - Predictability: The execution order is predictable, which can simplify the analysis and\n\
                understanding of system behavior.\n\
        - Simplicity: The implementation is straightforward and requires minimal overhead, making\n\
                it easy to understand and maintain.\n\n\
        Cons:\n\n\
        - Convoy Effect: Short processes stuck waiting behind long processes can cause the system\n\
                to become inefficient, known as the convoy effect.\n\
        - Starvation: If a process runs indefinitely, other processes won't have a chance to run,\n\
                leading to potential starvation.\n\
        - Poor Performance for Time-Sharing Systems: FCFS is not suitable for interactive or\n\
                time-sharing systems where responsiveness and fairness are important.\n\n\
        Use Cases:\n\n\
        - Batch Systems: FCFS is well-suited for batch processing systems where jobs are processed\n\
                in the order they arrive, and turnaround time is more important than response time.\n\
        - Simple and Predictable Environments: Environments where simplicity and predictability\n\
                are more valuable than efficiency, such as simple embedded systems or educational\n\
                environments.\n\
        - Non-Interactive Systems: Systems where processes do not require frequent interaction or\n\
                quick response times, as FCFS can handle a continuous flow of batch jobs\n\
                effectively.\n\n\n\
        By choosing the First Come First Served scheduling algorithm, you ensure a straightforward\n\
        and predictable approach to process scheduling. While it may not be the most efficient for\n\
        all scenarios, its simplicity and fairness can be advantageous in specific use cases,\n\
        providing a clear and easy-to-understand method for managing process execution in your\n\
        operating system.",
        0,
        BOOL,
        NULL
    }
};

struct Choice scheduler_choices[] = {
    {
        "Scheduling Algorithm",
        "Scheduling Algorithm\n\n\
        The Scheduling Algorithm configuration menu allows you to choose the algorithm used by\n\
        the operating system's scheduler to manage the execution of processes. The scheduler is a\n\
        critical component of the operating system that decides which process runs at any given\n\
        time, ensuring efficient CPU utilization and responsive system performance. Each scheduling\n\
        algorithm has its own characteristics, advantages, and disadvantages, making them suitable\n\
        for different types of workloads and system requirements.",
        scheduler_alg_configs,
        ARRAY_SIZE(scheduler_alg_configs)
    }
};
// ===============================================================================================



// ==============================Shell configurations=============================================
struct Config shell_configs[] = {
    {
        "CONFIG_SH_HISTORY",
        "Shell History",
        "Shell History\n\n\
        The Shell History Configuration setting allows you to manage how command history is\n\
        handled in your operating system's terminal. This feature enhances your productivity by\n\
        keeping track of previously executed commands, enabling you to easily recall them.\n\n\
        Enabling this configuration will introduce the 'history' command.",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_HISTORY_MAX_SIZE",
        "History Size",
        "History Size\n\n\
        The History Size Configuration setting allows you to specify the maximum number of commands\n\
        to be stored in the shell's history buffer. This determines how many previous commands are\n\
        available for recall",
        20,
        INT,
        "CONFIG_SH_HISTORY"
    }
#ifdef STEP_BY_STEP
    ,{
        "CONFIG_DONE",
        "Done",
        "Select when done with the configurations in the current menu in order to have access\n\
        to the next menu",
        0,
        BOOL,
        NULL
    }
#endif
};

struct Config shell_fgc_configs[] = {
    {
        "CONFIG_SH_FGC_BLACK",
        "Black",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_BLUE",
        "Blue",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_GREEN",
        "Green",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_CYAN",
        "Cyan",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_RED",
        "Red",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_MAGENTA",
        "Magenta",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_BROWN",
        "Brown",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_LIGHT_GREY",
        "Light Grey",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_DARK_GREY",
        "Dark Grey",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_LIGHT_BLUE",
        "Light Blue",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_LIGHT_GREEN",
        "Light Green",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_LIGHT_CYAN",
        "Light Cyan",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_LIGHT_RED",
        "Light Red",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_LIGHT_MAGENTA",
        "Light Magenta",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_LIGHT_BROWN",
        "Light Brown",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_FGC_WHITE",
        "White",
        "",
        1,
        BOOL,
        NULL
    }
};

struct Config shell_bgc_configs[] = {
    {
        "CONFIG_SH_BGC_BLACK",
        "Black",
        "",
        1,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_BLUE",
        "Blue",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_GREEN",
        "Green",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_CYAN",
        "Cyan",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_RED",
        "Red",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_MAGENTA",
        "Magenta",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_BROWN",
        "Brown",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_LIGHT_GREY",
        "Light Grey",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_DARK_GREY",
        "Dark Grey",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_LIGHT_BLUE",
        "Light Blue",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_LIGHT_GREEN",
        "Light Green",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_LIGHT_CYAN",
        "Light Cyan",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_LIGHT_RED",
        "Light Red",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_LIGHT_MAGENTA",
        "Light Magenta",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_LIGHT_BROWN",
        "Light Brown",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BGC_WHITE",
        "White",
        "",
        0,
        BOOL,
        NULL
    }
};

struct Choice shell_choices[] = {
    {
        "Terminal Background Color",
        "Terminal Background Color\n\n\
        The Terminal Background Color configuration allows you to set the default background\n\
        color for the terminal in your operating system.",
        shell_bgc_configs,
        ARRAY_SIZE(shell_bgc_configs)
    },

    {
        "Terminal Text Color",
        "Terminal Text Color\n\n\
        The Terminal Text Color configuration allows you to set the default text color for the\n\
        terminal in your operating system.",
        shell_fgc_configs,
        ARRAY_SIZE(shell_fgc_configs)
    }
};
// ===============================================================================================

char navigation_info[] = "Use the arrow keys navigate the menu. Enter selects menus/submenus (-->).\n\
  Pressing 'Y' includes/enables, 'N' excludes/disables features. Press 'q' to exit\n\
  Legend: (*) enabled, ( ) excluded";

struct Menu main_menu[] = {
    {
        "General Setup",
        "General Setup Menu\n\n\
        The General Setup Menu allows you to configure basic and essential settings\n\
        for your operating system. This includes fundamental options that define the\n\
        overall behavior and properties of the system.",
        general_setup_choices,
        general_setup_configs,
        ARRAY_SIZE(general_setup_choices),
        ARRAY_SIZE(general_setup_configs)
    },

    {
        "Memory Manager",
        "Memory Manager Menu\n\n\
        The Memory Manager is a critical component of the operating system responsible\n\
        for handling all aspects of memory allocation, management and protection. This menu\n\
        allows you to configure various settings that determine how memory is managed to\n\
        ensure efficient and secure operation of the system.",
        memory_manager_choices,
        memory_manager_configs,
        ARRAY_SIZE(memory_manager_choices),
        ARRAY_SIZE(memory_manager_configs)
    },

    {
        "Scheduler",
        "Scheduler Configurations\n\n\
        Another crucial part of the operating system is the scheduler, managing the execution\n\
        of processes by determining which process runs at any given time. This menu allows you\n\
        to configure various aspects of the scheduling algorithm, ensuring efficient CPU\n\
        utilization and responsive multitasking.",
        scheduler_choices,
        scheduler_configs,
        ARRAY_SIZE(scheduler_choices),
        ARRAY_SIZE(scheduler_configs)
    },

    {
        "Shell",
        "Shell Configurations\n\n\
        The shell is the command-line interface of the operating system, allowing you to\n\
        interact with the system through commands. This menu provides options to configure\n\
        aspects of the shell environment, enhancing usability, functionality and customization.",
        shell_choices,
        shell_configs,
        ARRAY_SIZE(shell_choices),
        ARRAY_SIZE(shell_configs)
    },

    {
        "Exit",
        "Exit\n\n\
        This is the exit. Exiting this menu will automatically save the configurations!",
        NULL,
        NULL,
        0,
        0
    },
};

#ifdef STEP_BY_STEP
char *main_menu_title = "myOS x86_32 Step-by-Step Kernel Configuration";
#else
char *main_menu_title = "myOS x86_32 Kernel Configuration";
#endif

void draw_window(WINDOW *win, int width, const char *title);
void display_message(WINDOW *win, const char *message, int x, int y, char *title);
void display_menu(WINDOW *menu_win, int highlight, struct Menu *choices, int n_choices, int x, int y);
void handle_general_setup_submenu(WINDOW *win, WINDOW *win2);
void display_submenu(WINDOW *win, struct Menu menu, WINDOW *win2);
void print_enabled_configurations(void);
int check_dependencies(struct Config config);

#endif

