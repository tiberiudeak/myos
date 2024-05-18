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
    Config *configs;
} Choice;

typedef struct {
    char *prompt;
    char *help;
    Choice *choices;
    Config *configs;
    int n_choices;
    int n_configs;
} Menu;

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
    {"Video Mode", video_mode_configs}
};

Menu main_menu[] = {
    {"General Setup",
    "General Setup help message",
    general_setup_choices,
    general_setup_configs,
    ARRAY_SIZE(general_setup_choices),
    ARRAY_SIZE(general_setup_configs)},

    {"Bla Setup",
    "Bla Setup help message",
    NULL,
    NULL,
    0,
    0},

    {"Exit",
    "Exit",
    NULL,
    NULL,
    0,
    0},
};

// Main menu choices
const char *choices[] = {
    "General Setup",
    "Memory Manager",
    "Scheduler",
    "Shell",
    "Exit"
};

const char *mm_choices_info[] = {
    "General Setup Menu\n\n\
 This menu allows you to configure basic and essential settings for the operating system\n\
 This includes fundamental options that define the overall behavior and properties of the system",
    "Memory Manager Menu",
    "Scheduler Menu",
    "Shell Menu",
    "Exit"
};

const char *general_setup_submenu[] = {
    "Video Mode",
    "Verbose",
    "RTC",
    "Back"
};

const char *general_setup_submenu_info[] = {
    "Video Mode",
    "Verbose Mode",
    "Real Time Clock",
    "Go back to the main menu"
};

const char *memory_manager_submenu[] = {
    "Algorithm",
    "Read after free prot.",
    "Back"
};

const char *memory_manager_submenu_info[] = {
    "User memory allocation algorithm",
    "Read after free protection",
    "Go back to the main menu"
};

const char *scheduler_submenu[] = {
    "Algorithm",
    "Back"
};

const char *scheduler_submenu_info[] = {
    "Scheduling Algorithm",
    "Go back to the main menu"
};

const char *shell_submenu[] = {
    "Hostname",
    "Shell history",
    "Terminal background color",
    "Terminal text color",
    "Back"
};

const char *shell_submenu_info[] = {
    "Hostname",
    "Enable Shell History",
    "Terminal background color",
    "Terminal text color",
    "Go back to the main menu"
};

void draw_window(WINDOW *win, int width, const char *title);
void display_message(WINDOW *win, const char *message, int x, int y);
void display_menu(WINDOW *menu_win, int highlight, Menu *choices, int n_choices, int x, int y);
void handle_general_setup_submenu(WINDOW *win, WINDOW *win2);
void display_submenu(WINDOW *win, Menu menu);

#endif

