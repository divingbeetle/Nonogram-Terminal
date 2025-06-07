#ifndef TUI_H
#define TUI_H

/******************************************************************************
 * Ncurses TUI functions.
 *
 * Function naming:
 * Display -> update/refresh the screen
 * Draw/print -> does not update the screen, only writes to buffer
 *****************************************************************************/

#include <menu.h>
#include <ncurses.h>

struct pos
{
    int y, x;
};

struct directional
{
    int top, right, bottom, left;
};

struct rect
{
    struct pos start, size;
};

#define POS_ZERO ((struct pos) {0, 0})
#define POS_NONE ((struct pos) {-1, -1})

struct pos  get_window_size(WINDOW *win);
struct pos  get_window_start(WINDOW *win);
struct rect get_window_rect(WINDOW *win);

struct pos get_subwin_start(WINDOW *win);

enum color_pairs
{
    /* Plain Color Pairs with black background */
    COLOR_P_DEFAULT,
    COLOR_P_RED,
    COLOR_P_GREEN,
    COLOR_P_YELLOW,
    COLOR_P_BLUE,
    COLOR_P_MAGENTA,
    COLOR_P_CYAN,
    COLOR_P_GREY,
    COLOR_P_N_PLAIN_PAIRS,

    /* Highlighted color pairs */
    COLOR_P_DEFAULT_HIGHLIGHTED,
    COLOR_P_RED_HIGHLIGHTED,
    COLOR_P_GREEN_HIGHLIGHTED,
    COLOR_P_YELLOW_HIGHLIGHTED,
    COLOR_P_BLUE_HIGHLIGHTED,
    COLOR_P_MAGENTA_HIGHLIGHTED,
    COLOR_P_CYAN_HIGHLIGHTED,
    COLOR_P_N_PAIRS
};

#define COLOR_GREY      8
#define COLOR_DARK_GREY 238

/**
 * Initialize/configure the ncurses screen.
 *  - Must be called before any other tui functions.
 *  - Installs cleanup function to be called at exit.
 */
void init_screen(void);

void print_in_middle(WINDOW *win, const char *string);

/**
 * Display a message to the user.
 *  - Message displayed until user presses any key.
 */
void display_notification(const char *msg);

void color_pairs_test(void);
void color_test(void);

#endif // TUI_H
