#include <ncurses.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void draw_window(WINDOW *win, int width, const char *title);
void display_message(WINDOW *win, const char *message);

int main() {
    initscr();
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

    const char *choices[] = { "General Setup" };
    int choice = 0;
    int highlight = 0;
    int n_choices = 1;

    while (1) {
        for (int i = 0; i < 1; ++i) {
            if (i == highlight) {
                wattron(win1, A_REVERSE);
            }

            mvwprintw(win1, i + 1, 1, "%s", choices[i]);
            wattroff(win1, A_REVERSE);
        }

        wrefresh(win1);

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
                if (highlight == 0) {
                    display_message(win2, "General setup is selected");
                }
                break;
            default:
                break;
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
    draw_window(win, getmaxx(win), "Info");
    mvwprintw(win, 1, 1, "%s", message);
    wrefresh(win);
}

