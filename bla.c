#include <ncurses.h>
#include <string.h>
#include "my_ncurses_menu.h"
#include <stdio.h>

int main(int argc, char *argv[]) {


    printf("test: %s\n", general_setup_configs[0].prompt);



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
    int n_choices = ARRAY_SIZE(choices);

    // print main menu in win1
    print_menu(win1, highlight, choices, n_choices);
    keypad(win1, TRUE);

    // display info message for the highlighted choice in win1
    display_message(win2, mm_choices_info[highlight-1]);

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

                if (strcmp(choices[choice - 1], "General Setup") == 0) {
                    handle_general_setup_submenu(win1, win2);
                }

                break;
            default:
                break;
        }

        print_menu(win1, highlight, choices, n_choices);
        display_message(win2, mm_choices_info[highlight-1]);

        if (choice != 0) {
            if (choice == n_choices) {
                endwin();
                return(0);
            }
            else {
                // process choice
                //display_message(win2, "Something is selected is selected");
                choice = 0;
            }
        }
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

void display_message(WINDOW *win, const char *message) {
    werase(win);
    mvwprintw(win, 1, 1, "%s", message);
    draw_window(win, getmaxx(win), "Info");
    wrefresh(win);
}

void print_menu(WINDOW *menu_win, int highlight, const char *choices[], int n_choices) {
    int x = 2, y = 2;
    draw_window(menu_win, getmaxx(menu_win), "Step-by-step configuration for myOS x86_32");

    for (int i = 0; i < n_choices; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        } else
            mvwprintw(menu_win, y, x, "%s", choices[i]);
        ++y;
    }

    wrefresh(menu_win);
}

void handle_general_setup_submenu(WINDOW *win1, WINDOW *win2) {
    int n_choices = ARRAY_SIZE(general_setup_submenu);
    int highlight = 1, choice = 0;

    int height, width;
    getmaxyx(win1, height, width);

    WINDOW *general_setup_win = newwin(height, width, 0, 0);
    draw_window(general_setup_win, width, "General Setup");

    print_menu(general_setup_win, highlight, general_setup_submenu, n_choices);
    keypad(general_setup_win, TRUE);

    // display info message for the highlighted choice in win1
    display_message(win2, general_setup_submenu_info[highlight-1]);

    while (1) {
        int ch = wgetch(win1);

        switch (ch) {
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

        print_menu(general_setup_win, highlight, general_setup_submenu, n_choices);
        display_message(win2, general_setup_submenu_info[highlight-1]);

        if (choice != 0) {
            if (choice == n_choices) {
                endwin();
                return;
            }

            else {
                // process choice
                if (strcmp(general_setup_submenu[choice - 1], "Video Mode") == 0) {
                }
                choice = 0;
            }
        }
    }
}

