#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_ncurses_menu.h"

int main(int argc, char *argv[]) {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);

    int height, width;
    getmaxyx(stdscr, height, width);

    int half_width = width / 2;

    WINDOW *win1 = newwin(height, half_width, 0, 0);
    WINDOW *win2 = newwin(height, half_width, 0, half_width);

    draw_window(win1, half_width, main_menu_title);
    draw_window(win2, half_width, "Info");

    int choice = 0;
    int highlight = 1;

    // if step by step, only the first menu is visible from the start
#ifdef STEP_BY_STEP
    int n_choices = 1;
#else
    int n_choices = ARRAY_SIZE(main_menu);
#endif

    // print main menu in win1
    display_menu(win1, highlight, main_menu, n_choices, MAIN_MENU_X, MAIN_MENU_Y);
    keypad(win1, TRUE);

    // display info message for the highlighted choice in win1
    display_message(win2, main_menu[highlight-1].help, SEC_MENU_X, SEC_MENU_Y, "Info");

    while (1) {
        int ch = wgetch(win1);

        switch (ch) {
            case 'q':
                delwin(win1);
                delwin(win2);
                endwin();

                print_enabled_configurations();

                return 0;
            case KEY_UP:
                if (highlight == 1)
                    highlight = n_choices;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_choices)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case 10:
                choice = highlight;
                break;
            default:
                break;
        }

#ifdef STEP_BY_STEP
        if (choice != 0) {
            if (strcmp(main_menu[choice-1].prompt, "Exit") == 0) {
                endwin();
                print_enabled_configurations();

                return 0;
            }

            display_submenu(win1, main_menu[choice-1], win2);
            choice = 0;
        }
#else
        if (choice != 0) {
            if (choice == n_choices) {
                endwin();
                print_enabled_configurations();

                return 0;
            }

            // process choice
            display_submenu(win1, main_menu[choice-1], win2);
            choice = 0;
        }
#endif

#ifdef STEP_BY_STEP
    n_choices = 1;

    for (int i = 0; i < ARRAY_SIZE(main_menu); i++) {
        for (int j = 0; j < main_menu[i].n_configs; j++) {
            if (main_menu[i].configs[j].prompt != NULL &&
                strcmp(main_menu[i].configs[j].prompt, "Done") == 0 &&
                main_menu[i].configs[j].default_val == 1) {
                n_choices++;
            }
        }
    }
#endif

        display_menu(win1, highlight, main_menu, n_choices, MAIN_MENU_X, MAIN_MENU_Y);
        display_message(win2, main_menu[highlight-1].help, SEC_MENU_X, SEC_MENU_Y, "Info");

    }

    delwin(win1);
    delwin(win2);
    endwin();
    return 0;
}

void draw_window(WINDOW *win, int width, const char *title) {
    werase(win);

    // print navigation help only in the main window
    if (win->_begx == 0 && win->_begy == 0)
        mvwprintw(win, 2, 2, "%s", navigation_info);

    box(win, 0, 0);
    int startx = (width - strlen(title)) / 2;
    mvwprintw(win, 0, startx, "%s", title);
    wrefresh(win);
}

void display_message(WINDOW *win, const char *message, int x, int y, char *title) {
    werase(win);
    draw_window(win, getmaxx(win), title);
    mvwprintw(win, y, x, "%s", message);
    box(win, 0, 0);
    mvwprintw(win, 0, (getmaxx(win) - strlen(title)) / 2, "%s", title);
    wrefresh(win);
}

void display_message2(WINDOW *win, Config config, int x, int y, char *extra) {

    if (config.type == BOOL && config.default_val == 1) {
        if (extra)
            mvwprintw(win, y, x, "(*) %s %s", config.prompt, extra);
        else
            mvwprintw(win, y, x, "(*) %s", config.prompt);
    }
    else if (config.type == INT) {
        if (extra)
            mvwprintw(win, y, x, "(%d) %s %s", config.default_val, config.prompt, extra);
        else
            mvwprintw(win, y, x, "(%d) %s", config.default_val, config.prompt);
    }
    else {
        if (extra)
            mvwprintw(win, y, x, "( ) %s %s", config.prompt, extra);
        else
            mvwprintw(win, y, x, "( ) %s", config.prompt);
    }
}

void display_configs(WINDOW *win, int highlight, Config *configs, int n_configs,
                        int n_choices, int x, int y) {

    for (int i = 0; i < n_configs; ++i) {
        if (highlight == i + 1 + n_choices) {
            wattron(win, A_REVERSE);

            if (check_dependencies(configs[i])) {
                display_message2(win, configs[i], x, y, NULL);
            }
            else {
                if (has_colors() && start_color() == OK) {
                    init_pair(1, COLOR_RED, COLOR_BLACK);
                    wattron(win, COLOR_PAIR(1));

                    display_message2(win, configs[i], x, y, "(unmet dependencies)");

                    wattroff(win, COLOR_PAIR(1));
                }
                else {
                    display_message2(win, configs[i], x, y, "(unmet dependencies)");
                }
            }

            wattroff(win, A_REVERSE);
        }
        else {
            if (check_dependencies(configs[i])) {
                display_message2(win, configs[i], x, y, NULL);
            }
            else {
                if (has_colors() && start_color() == OK) {
                    init_pair(1, COLOR_RED, COLOR_BLACK);
                    wattron(win, COLOR_PAIR(1));

                    display_message2(win, configs[i], x, y, "(unmet dependencies)");

                    wattroff(win, COLOR_PAIR(1));
                }
                else {
                    display_message2(win, configs[i], x, y, "(unmet dependencies)");
                }
            }
        }

        ++y;
    }

    wrefresh(win);
}

void display_choices(WINDOW *win, int highlight, Choice *choices, int n_choices, int x, int y) {
    for (int i = 0; i < n_choices; ++i) {
        if (highlight == i + 1) {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, x, "%s -->", choices[i].prompt);
            wattroff(win, A_REVERSE);
        }
        else {
            mvwprintw(win, y, x, "%s -->", choices[i].prompt);
        }
        ++y;
    }

    wrefresh(win);
}

void display_menu(WINDOW *win, int highlight, Menu *entries, int n_entries, int x, int y) {
    draw_window(win, getmaxx(win), main_menu_title);

    for (int i = 0; i < n_entries; ++i) {
        if (highlight == i + 1) {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, x, "%s -->", entries[i].prompt);
            wattroff(win, A_REVERSE);
        } else
            mvwprintw(win, y, x, "%s -->", entries[i].prompt);
        ++y;
    }

    wrefresh(win);
}

void display_choice(WINDOW *win, Choice choice, WINDOW *win2) {

    int height, width;
    getmaxyx(win, height, width);

    WINDOW *choice_win = newwin(height / 2, width, height / 2, 0);

    draw_window(choice_win, width, choice.prompt);
    keypad(choice_win, TRUE);

    int highlight = 1;

    display_configs(choice_win, highlight, choice.configs,
            choice.n_configs, 0, SEC_MENU_X, SEC_MENU_Y);

    display_message(win2, choice.configs[0].help_message, SEC_MENU_X, SEC_MENU_Y, "Info");

    wrefresh(choice_win);

    while (1) {
        int ch = wgetch(choice_win);

        switch (ch) {
            case 'q':
                delwin(choice_win);
                return;
            case KEY_UP:
                if (highlight == 1)
                    highlight = choice.n_configs;
                else
                    --highlight;

                break;
            case KEY_DOWN:
                if (highlight == choice.n_configs)
                    highlight = 1;
                else
                    ++highlight;

                break;
            case 121:
                // 'y'
                choice.configs[highlight - 1].default_val = 1;

                // disable all other configs
                for (int i = 0; i < choice.n_configs; i++) {
                    if (i != highlight - 1 && choice.configs[i].default_val == 1)
                        choice.configs[i].default_val = 0;
                }

                break;
            default:
                break;
        }

        display_configs(choice_win, highlight, choice.configs,
                choice.n_configs, 0, SEC_MENU_X, SEC_MENU_Y);
        display_message(win2, choice.configs[highlight-1].help_message, SEC_MENU_X, SEC_MENU_Y, "Info");
    }
}

void display_int_window(WINDOW *win, Config *config) {

    int height, width;
    getmaxyx(win, height, width);

    WINDOW *int_win = newwin(height / 2, width, height / 2, 0);
    curs_set(1);

    draw_window(int_win, width, config->prompt);
    display_message(int_win, "Insert number: ", SEC_MENU_X, SEC_MENU_Y, config->prompt);

    wrefresh(int_win);
    int x = 17, y = 2, index = 0;
    char buffer[12] = {0};

    while (1) {
        int ch = wgetch(int_win);

        if (ch != 10) {
            if (ch >= '0' && ch <= '9') {
                mvwprintw(int_win, y, x++, "%c", ch);
                // TODO: store characters in a buffer and convert it later to int
                buffer[index++] = ch;
            }
        }
        else {
            // convert buffer to int
            buffer[index] = '\0';
            config->default_val = atoi(buffer);

            delwin(int_win);
            curs_set(0);
            return;
        }
    }
}

void display_submenu(WINDOW *win, Menu menu, WINDOW *win2) {
    int height, width;
    getmaxyx(win, height, width);

    WINDOW *new_window = newwin(height, width, 0, 0);
    curs_set(0);

    draw_window(new_window, width, menu.prompt);
    keypad(new_window, TRUE);

    int highlight = 1;
    int choice = 0;

    if (menu.choices == NULL && menu.configs == NULL)
        return;

    if (menu.choices != NULL) {
        display_choices(new_window, highlight, menu.choices, menu.n_choices,
                MAIN_MENU_X, MAIN_MENU_Y);
        display_message(win2, menu.choices[0].help_message, SEC_MENU_X, SEC_MENU_Y, "Info");
    }

    if (menu.configs != NULL && menu.choices != NULL)
        display_configs(new_window, highlight, menu.configs, menu.n_configs,
                menu.n_choices, MAIN_MENU_X, menu.n_choices + MAIN_MENU_Y);
    else {
        display_configs(new_window, highlight, menu.configs, 0, menu.n_configs,
                MAIN_MENU_X, MAIN_MENU_Y);
        display_message(win2, menu.configs[0].help_message, SEC_MENU_X, SEC_MENU_Y, "Info");
    }

    int total_entries = menu.n_choices + menu.n_configs;

    wrefresh(new_window);

    while (1) {
        int ch = wgetch(new_window);

        switch (ch) {
            case 'q':
                delwin(new_window);
                return;
            case KEY_UP:
                if (highlight == 1)
                    highlight = total_entries;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == total_entries)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case 121:
                // 'y'
                if (highlight > menu.n_choices &&
                        check_dependencies(menu.configs[highlight - 1 - menu.n_choices])) {
                    menu.configs[highlight - 1 - menu.n_choices].default_val = 1;
                }

                break;
            case 110:
                // 'n'
                if (highlight > menu.n_choices &&
                        check_dependencies(menu.configs[highlight - 1 - menu.n_choices])) {
#ifdef STEP_BY_STEP
                    // once 'Done' is selected, if cannot be disabled
                    if (strcmp(menu.configs[highlight - 1 - menu.n_choices].prompt, "Done")
                            != 0)
                    menu.configs[highlight - 1 - menu.n_choices].default_val = 0;
#else
                    menu.configs[highlight - 1 - menu.n_choices].default_val = 0;
#endif
                }

                break;
            case 10:
                choice = highlight;
                break;
            default:
                break;
        }

        if (choice != 0) {
            // process choice
            if (choice <= menu.n_choices) {
                // new window for the choice
                display_choice(win, menu.choices[choice-1], win2);
            }
            else if (menu.configs[choice - 1 - menu.n_choices].type != BOOL) {
                // if type is INT or STRING
                if (check_dependencies(menu.configs[choice - 1 - menu.n_choices]))
                    display_int_window(win, &menu.configs[choice - 1 - menu.n_choices]);
            }

            choice = 0;
        }

        draw_window(new_window, width, menu.prompt);
        display_choices(new_window, highlight, menu.choices, menu.n_choices,
                            MAIN_MENU_X, MAIN_MENU_Y);
        display_configs(new_window, highlight, menu.configs, menu.n_configs,
                            menu.n_choices, MAIN_MENU_X, menu.n_choices + MAIN_MENU_Y);

        if (highlight <= menu.n_choices) {
            display_message(win2, menu.choices[highlight - 1].help_message,
                    SEC_MENU_X, SEC_MENU_Y, "Info");
        }
        else {
            display_message(win2, menu.configs[highlight - 1 - menu.n_choices].help_message,
                    SEC_MENU_X, SEC_MENU_Y, "Info");
        }
    }
}

void print_enabled_configurations(void) {
    FILE *fp = fopen(".config", "w");

    if (fp == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    int n = ARRAY_SIZE(main_menu);

    for (int i = 0; i < n; i++) {
        Menu current = main_menu[i];

        // get configurations from choices
        for (int j = 0; j < current.n_choices; j++) {
            Choice c = current.choices[j];

            for (int k = 0; k < c.n_configs; k++) {
                if (check_dependencies(c.configs[k])) {
                    if (c.configs[k].default_val == 1 && c.configs[k].type == BOOL) {
                        fprintf(fp, "%s=y\n", c.configs[k].symbol);
                        printf("%s=y\n", c.configs[k].symbol);
                    }
                    else if (c.configs[k].type == INT) {
                        fprintf(fp, "%s=%d\n", c.configs[k].symbol, c.configs[k].default_val);
                        printf("%s=%d\n", c.configs[k].symbol, c.configs[k].default_val);
                    }
                }
            }
        }

        // get configurations
        for (int j = 0; j < current.n_configs; j++) {
            if (check_dependencies(current.configs[j])) {
                if (current.configs[j].default_val == 1 && current.configs[j].type == BOOL) {
                    fprintf(fp, "%s=y\n", current.configs[j].symbol);
                    printf("%s=y\n", current.configs[j].symbol);
                }
                else if (current.configs[j].type == INT) {
                    fprintf(fp, "%s=%d\n", current.configs[j].symbol, current.configs[j].default_val);
                    printf("%s=%d\n", current.configs[j].symbol, current.configs[j].default_val);
                }
            }
        }
    }
}

// checks if the given config is enabled or not
int check_config(char *config) {
    int n = ARRAY_SIZE(main_menu);

    for (int i = 0; i < n; i++) {
        Menu current = main_menu[i];

        // get configurations from choices
        for (int j = 0; j < current.n_choices; j++) {
            Choice c = current.choices[j];

            for (int k = 0; k < c.n_configs; k++) {
                if (strcmp(c.configs[k].symbol, config) == 0) {
                    if (c.configs[k].default_val != 0)
                        return 1;

                    return 0;
                }
            }
        }

        // get configurations
        for (int j = 0; j < current.n_configs; j++) {
            if (strcmp(current.configs[j].symbol, config) == 0) {
                if (current.configs[j].default_val != 0)
                    return 1;

                return 0;
            }
        }
    }

    return 0;
}

int check_dependencies(Config config) {
    const char delim[] = ",";
    char *token;

    if (config.deps == NULL)
        return 1;

    token = strtok(config.deps, delim);

    while (token != NULL) {
        if (check_config(token) == 0)
            return 0;

        token = strtok(NULL, delim);
    }

    return 1;
}

