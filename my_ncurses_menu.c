#include <ncurses.h>
#include <string.h>
#include "my_ncurses_menu.h"
#include <stdio.h>

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

    draw_window(win1, half_width, "Step-by-step configuration for myOS x86_32");
    draw_window(win2, half_width, "Info");

    int choice = 0;
    int highlight = 1;
    int n_choices = ARRAY_SIZE(main_menu);

    // print main menu in win1
    display_menu(win1, highlight, main_menu, n_choices, 2, 2);
    keypad(win1, TRUE);

    // display info message for the highlighted choice in win1
    display_message(win2, main_menu[highlight-1].help, 1, 1);

    while (1) {
        int ch = wgetch(win1);

        switch (ch) {
            case 'q':
                delwin(win1);
                delwin(win2);
                endwin();

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

        if (choice != 0) {
            if (choice == n_choices) {
                endwin();
                return(0);
            }
            else {
                // process choice
                //display_message(win2, "Something is selected", 1, 1);
                display_submenu(win1, main_menu[choice-1]);
                choice = 0;
            }
        }

        display_menu(win1, highlight, main_menu, n_choices, 2, 2);
        display_message(win2, main_menu[highlight-1].help, 1, 1);
    }

    delwin(win1);
    delwin(win2);
    endwin();
    return 0;
}

void draw_window(WINDOW *win, int width, const char *title) {
    box(win, 0, 0);
    int startx = (width - strlen(title)) / 2;
    mvwprintw(win, 0, startx, "%s", title);
    wrefresh(win);
}

void display_message(WINDOW *win, const char *message, int x, int y) {
    werase(win);
    mvwprintw(win, y, x, "%s", message);
    draw_window(win, getmaxx(win), "Info");
    wrefresh(win);
}

void display_configs(WINDOW *win, int highlight, Config *configs, int n_configs, int x, int y) {
    for (int i = 0; i < n_configs; ++i) {
        if (highlight == i + 2) {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, x, "%s", configs[i].prompt);
            wattroff(win, A_REVERSE);
        }
        else {
            mvwprintw(win, y, x, "%s", configs[i].prompt);
        }
        ++y;
    }

    wrefresh(win);
}

void display_choices(WINDOW *win, int highlight, Choice *choices, int n_choices, int x, int y) {
    for (int i = 0; i < n_choices; ++i) {
        if (highlight == i + 1) {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, x, "%s", choices[i].prompt);
            wattroff(win, A_REVERSE);
        }
        else {
            mvwprintw(win, y, x, "%s", choices[i].prompt);
        }
        ++y;
    }

    wrefresh(win);
}

void display_menu(WINDOW *win, int highlight, Menu entries[], int n_entries, int x, int y) {
    draw_window(win, getmaxx(win), "Step-by-step configuration for myOS x86_32");

    for (int i = 0; i < n_entries; ++i) {
        if (highlight == i + 1) {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, x, "%s", entries[i].prompt);
            wattroff(win, A_REVERSE);
        } else
            mvwprintw(win, y, x, "%s", entries[i].prompt);
        ++y;
    }

    wrefresh(win);
}

void display_submenu(WINDOW *win, Menu menu) {
    int height, width;
    getmaxyx(win, height, width);

    WINDOW *new_window = newwin(height, width, 0, 0);

    draw_window(new_window, width, menu.prompt);
    keypad(new_window, TRUE);

    int highlight = 1;
    int choice = 0;
    display_choices(new_window, highlight, menu.choices, menu.n_choices, 2, 2);
    display_configs(new_window, highlight, menu.configs, menu.n_configs, 2, menu.n_choices + 2);

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
            case 10:
                choice = highlight;
                break;
            default:
                break;
        }

        display_choices(new_window, highlight, menu.choices, menu.n_choices, 2, 2);
        display_configs(new_window, highlight, menu.configs, menu.n_configs, 2, menu.n_choices + 2);
    }
}
