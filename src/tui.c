#include "tui.h"
#include "utils.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define KEY_LF 10

/* Function Prototypes */

void end_screen(void);
void init_colors(void);
void set_custom_colors(void);
void set_color_pairs(void);

/**
 * @param  color Ncurses color number to set
 * @param  r, g, b RGB values to set the color (0-255)
 */
void set_color_rgb(short color, short r, short g, short b);
bool enough_screen_size(struct pos required_size)
{
    struct pos screen_size;
    getmaxyx(stdscr, screen_size.y, screen_size.x);
    return (screen_size.y >= required_size.y && screen_size.x >= required_size.x);
}

/* Public */

void color_pairs_test(void)
{
    for (int i = 0; i < COLOR_P_N_PAIRS; i++)
    {
        wattron(stdscr, COLOR_PAIR(i));
        mvprintw(i, 0, "Color pair %d", i);
        wattroff(stdscr, COLOR_PAIR(i));
    }
}
void color_test(void)
{
    int n_colors = COLORS;
    for (int i = 0; i < n_colors; i++)
    {
        init_pair(i, i, COLOR_BLACK);
        wattron(stdscr, COLOR_PAIR(i));
        mvprintw(i, 0, "Color %d", i);
        wattroff(stdscr, COLOR_PAIR(i));
    }
}

void init_screen(void)
{
    initscr();
    atexit(end_screen);

    cbreak();
    noecho();
    keypad(stdscr, true);
    curs_set(0);
    init_colors();
}

void print_in_middle(WINDOW *win, const char *string)
{
    struct pos size;
    getmaxyx(win, size.y, size.x);
    mvwprintw(win, size.y / 2, (size.x - strlen(string)) / 2, "%s", string);
}

void display_notification(const char *msg)
{
    clear();
    print_in_middle(stdscr, msg);
    refresh();
    getch();
    clear();
    refresh();
}


struct pos get_window_size(WINDOW *win)
{
    struct pos size = POS_ZERO;
    getmaxyx(win, size.y, size.x);
    return size;
}

struct pos get_window_start(WINDOW *win)
{
    struct pos start = POS_ZERO;
    getbegyx(win, start.y, start.x);
    return start;
}

struct rect get_window_rect(WINDOW *win)
{
    struct rect rect;
    rect.start = get_window_start(win);
    rect.size  = get_window_size(win);

    return rect;
}

struct pos get_subwin_start(WINDOW *win)
{
    struct pos start = POS_ZERO;
    getparyx(win, start.y, start.x);
    return start;
}
/* Private */

void end_screen(void) 
{ 
    endwin();
}

void set_custom_colors(void)
{
    set_color_rgb(COLOR_BLACK, 30, 33, 34);
    set_color_rgb(COLOR_RED, 236, 107, 100);
    set_color_rgb(COLOR_GREEN, 169, 182, 101);
    set_color_rgb(COLOR_YELLOW, 224, 192, 128);
    set_color_rgb(COLOR_BLUE, 102, 144, 235);
    set_color_rgb(COLOR_MAGENTA, 211, 134, 155);
    set_color_rgb(COLOR_CYAN, 125, 174, 163);
    set_color_rgb(COLOR_WHITE, 192, 177, 150);
    if (COLORS >= 8)
    {
        set_color_rgb(COLOR_GREY, 153, 153, 153);
        set_color_rgb(COLOR_RED + 8, 255, 0, 0);
        set_color_rgb(COLOR_GREEN + 8, 0, 255, 0);
        set_color_rgb(COLOR_YELLOW + 8, 255, 255, 0);
        set_color_rgb(COLOR_BLUE + 8, 0, 0, 255);
        set_color_rgb(COLOR_MAGENTA + 8, 255, 0, 255);
        set_color_rgb(COLOR_CYAN + 8, 0, 255, 255);
        set_color_rgb(COLOR_WHITE + 8, 235, 219, 178);
    }
    if (COLORS >= 16)
    {
        set_color_rgb(COLOR_DARK_GREY, 54, 57, 58);
    }
}

void set_color_rgb(short color, short r, short g, short b)
{
    // ncurses init_color() takes 0-1000 color values 
    assert(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255);

    float r_ratio = ((float)r) / 255.0;
    float g_ratio = ((float)g) / 255.0;
    float b_ratio = ((float)b) / 255.0;

    r = (short)(r_ratio * 1000.0);
    g = (short)(g_ratio * 1000.0);
    b = (short)(b_ratio * 1000.0);
    init_color(color, r, g, b);
}

void set_color_pairs(void)
{
    init_pair(COLOR_P_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_P_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_P_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_P_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_P_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_P_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_P_CYAN, COLOR_CYAN, COLOR_BLACK);

    init_pair(COLOR_P_DEFAULT_HIGHLIGHTED, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_P_RED_HIGHLIGHTED, COLOR_BLACK, COLOR_RED);
    init_pair(COLOR_P_GREEN_HIGHLIGHTED, COLOR_BLACK, COLOR_GREEN);
    init_pair(COLOR_P_YELLOW_HIGHLIGHTED, COLOR_BLACK, COLOR_YELLOW);
    init_pair(COLOR_P_BLUE_HIGHLIGHTED, COLOR_BLACK, COLOR_BLUE);
    init_pair(COLOR_P_MAGENTA_HIGHLIGHTED, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(COLOR_P_CYAN_HIGHLIGHTED, COLOR_BLACK, COLOR_CYAN);

    if (COLORS >= 255)
    {
        init_pair(COLOR_P_DEFAULT_HIGHLIGHTED, COLOR_WHITE, COLOR_DARK_GREY);
        init_pair(COLOR_P_RED_HIGHLIGHTED, COLOR_RED, COLOR_DARK_GREY);
        init_pair(COLOR_P_GREEN_HIGHLIGHTED, COLOR_GREEN, COLOR_DARK_GREY);
        init_pair(COLOR_P_YELLOW_HIGHLIGHTED, COLOR_YELLOW, COLOR_DARK_GREY);
        init_pair(COLOR_P_BLUE_HIGHLIGHTED, COLOR_BLUE, COLOR_DARK_GREY);
        init_pair(COLOR_P_MAGENTA_HIGHLIGHTED, COLOR_MAGENTA, COLOR_DARK_GREY);
        init_pair(COLOR_P_CYAN_HIGHLIGHTED, COLOR_CYAN, COLOR_DARK_GREY);
    }

}

void init_colors(void)
{
    if (!has_colors())
    {
        LOG(LOG_WARNING, "Terminal does not support colors");
        return;
    }

    start_color();
    LOGF(LOG_INFO , "Colors: %d", COLORS);
    if (can_change_color())
    {
        LOG(LOG_INFO, "Can change colors");
        set_custom_colors();
    }
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);

    set_color_pairs();
}


