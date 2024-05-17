#include <ncurses.h>
#include <stdlib.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices);
void process_choice(int choice);

int main() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0); // Hide the cursor

    int startx = 0, starty = 0;
    int width = 30, height = 10;
    int highlight = 1;
    int choice = 0;
    int c;

    char *choices[] = {
        "General Setup",
        "Memory Manager",
        "Scheduler",
        "Shell",
        "Exit",
    };
    int n_choices = ARRAY_SIZE(choices);

    WINDOW *menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE);
    refresh();

    print_menu(menu_win, highlight, choices, n_choices);

    while (1) {
        c = wgetch(menu_win);
        switch (c) {
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
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }

        print_menu(menu_win, highlight, choices, n_choices);

        if (choice != 0) { // User made a choice
            if (choice == n_choices) {
                endwin();
                exit(0);
            } else {
                process_choice(choice);
                choice = 0; // Reset choice after processing
            }
        }
    }

    endwin();
    return 0;
}

void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices) {
    int x = 2, y = 2;
    box(menu_win, 0, 0);

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

void process_choice(int choice) {
    // Placeholder for processing different menu choices
    // You can add sub-menus or actions based on the choice
    clear();
    mvprintw(0, 0, "You selected option %d", choice);
    refresh();
    getch(); // Wait for user input
}

