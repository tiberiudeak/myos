#ifndef MY_NCURSES_MENU_H
#define MY_NCURSES_MENU_H 1

#include <ncurses.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef enum {
    INT, BOOL
} type;

typedef struct {
    char *symbol;
    char *prompt;
    char *help_message;
    int default_val;
    type type;
    char *deps;
} Config;

typedef struct {
    char *prompt;
    char *help_message;
    Config *configs;
    int n_configs;
} Choice;

typedef struct {
    char *prompt;
    char *help;
    Choice *choices;
    Config *configs;
    int n_choices;
    int n_configs;
} Menu;

// ==============================General Setup configurations=====================================
Config general_setup_configs[] = {
    {"CONFIG_TTY_VBE_WIDTH", "Graphics Mode Width", "Help message", 1920, INT, "CONFIG_TTY_VBE"},
    {"CONFIG_TTY_VBE_HEIGHT", "Graphics Mode Height", "Help message", 1080, INT, "CONFIG_TTY_VBE"},
    {"CONFIG_VERBOSE", "Verbose", "Help message", 0, BOOL, NULL},
    {"CONFIG_RTC", "Real time clock", "Enable the Real Time Clock", 1, BOOL, NULL}
};

Config video_mode_configs[] = {
    {"CONFIG_TTY_VGA", "VGA Text Mode", "Help message", 1, BOOL, NULL},
    {"CONFIG_TTY_VBE", "Graphics Mode", "Help message", 0, BOOL, NULL}
};

Choice general_setup_choices[] = {
    {"Video Mode", "Help message video mode", video_mode_configs, ARRAY_SIZE(video_mode_configs)}
};
// ===============================================================================================



// ==============================Memory Manager configurations====================================
Config memory_manager_configs[] = {
    {"CONFIG_READ_AFTER_FREE_PROT",
     "Read after free protection",
     "Help message",
     1,
     BOOL,
     NULL
    }
};

Config mem_alloc_alg_configs[] = {
    {"CONFIG_UVMM_BESTFIT", "Best Fit", "Help message", 1, BOOL, NULL},
    {"CONFIG_UVMM_FIRSTFIT", "First Fit", "Help message", 0, BOOL, NULL},
    {"CONFIG_UVMM_NEXTFIT", "Next Fit", "Help message", 0, BOOL, NULL},
    {"CONFIG_UVMM_WORSTFIT", "Worst Fit", "Help message", 0, BOOL, NULL}
};

Choice memory_manager_choices[] = {
    {"Memory Allocation Algorithm", "Help message", mem_alloc_alg_configs, ARRAY_SIZE(mem_alloc_alg_configs)}
};
// ===============================================================================================



// ==============================Scheduler configurations=========================================
Config scheduler_configs[] = {};

Config scheduler_alg_configs[] = {
    {
        "CONFIG_ROUND_ROBIN",
        "Round-Robin",
        "Help message",
        1,
        BOOL,
        NULL
    },

    {
        "CONFIG_SIMPLE_SCH",
        "Simple",
        "Help message",
        0,
        BOOL,
        NULL
    }
};

Choice scheduler_choices[] = {
    {
        "Scheduling Algorithm",
        "Help message",
        scheduler_alg_configs,
        ARRAY_SIZE(scheduler_alg_configs)
    }
};
// ===============================================================================================



// ==============================Shell configurations=============================================
Config shell_configs[] = {
    {
        "CONFIG_SH_HISTORY",
        "Shell History",
        "Help message",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_HISTORY_MAX_SIZE",
        "History Size",
        "Help message",
        20,
        INT,
        "CONFIG_SH_HISTORY"
    }
};

Config shell_fgc_configs[] = {
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

Config shell_bkc_configs[] = {
    {
        "CONFIG_SH_BCK_BLACK",
        "Black",
        "",
        1,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_BLUE",
        "Blue",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_GREEN",
        "Green",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_CYAN",
        "Cyan",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_RED",
        "Red",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_MAGENTA",
        "Magenta",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_BROWN",
        "Brown",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_LIGHT_GREY",
        "Light Grey",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_DARK_GREY",
        "Dark Grey",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_LIGHT_BLUE",
        "Light Blue",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_LIGHT_GREEN",
        "Light Green",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_LIGHT_CYAN",
        "Light Cyan",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_LIGHT_RED",
        "Light Red",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_LIGHT_MAGENTA",
        "Light Magenta",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_LIGHT_BROWN",
        "Light Brown",
        "",
        0,
        BOOL,
        NULL
    },

    {
        "CONFIG_SH_BCK_WHITE",
        "White",
        "",
        0,
        BOOL,
        NULL
    }
};

Choice shell_choices[] = {
    {
        "Terminal Background Color",
        "Help message",
        shell_bkc_configs,
        ARRAY_SIZE(shell_bkc_configs)
    },

    {
        "Terminal Text Color",
        "Help message",
        shell_fgc_configs,
        ARRAY_SIZE(shell_fgc_configs)
    }
};
// ===============================================================================================



Menu main_menu[] = {
    {"General Setup",
    "General Setup help message",
    general_setup_choices,
    general_setup_configs,
    ARRAY_SIZE(general_setup_choices),
    ARRAY_SIZE(general_setup_configs)},

    {"Memory Manager",
    "Memory Manager",
    memory_manager_choices,
    memory_manager_configs,
    ARRAY_SIZE(memory_manager_choices),
    ARRAY_SIZE(memory_manager_configs)},

    {"Scheduler",
    "Scheduler configurations",
    scheduler_choices,
    scheduler_configs,
    ARRAY_SIZE(scheduler_choices),
    ARRAY_SIZE(scheduler_configs)},

    {"Shell",
    "Shell configurations",
    shell_choices,
    shell_configs,
    ARRAY_SIZE(shell_choices),
    ARRAY_SIZE(shell_configs)},

    {"Exit",
    "Exit",
    NULL,
    NULL,
    0,
    0},
};

void draw_window(WINDOW *win, int width, const char *title);
void display_message(WINDOW *win, const char *message, int x, int y, char *title);
void display_menu(WINDOW *menu_win, int highlight, Menu *choices, int n_choices, int x, int y);
void handle_general_setup_submenu(WINDOW *win, WINDOW *win2);
void display_submenu(WINDOW *win, Menu menu, WINDOW *win2);
void print_enabled_configurations(void);
int check_dependencies(Config config);

#endif

