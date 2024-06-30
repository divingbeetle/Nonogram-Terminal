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

#define COLOR_GREY 8
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

/* ---- Menu ---- */ 

/**
 * @TODO: Currnet implementation is a minimal working model.
 *        Menu design and possible requirements are not considered yet.
 */

#define MENU_NOT_SELECTED -1

struct menu_param
{
    char *title;
    char **choices;
    char **descriptions;
    int n_choices;
    struct pos start;
    struct pos size;
};

struct menu_config
{
    bool rescale;
};
extern const struct menu_config menu_config_default;

struct menu_set
{
    const struct menu_param *params;
    WINDOW *win;
    MENU *menu;
    ITEM **items;
};

struct menu_set *menu_set_create(const struct menu_param *params);

/**
 * @param config Use global menu_config_default for default values.
 */
void menu_set_configure(struct menu_set *mset, const struct menu_config config);
void menu_set_destroy(struct menu_set *mset);

/**
 * Prompt the user to select a choice from the menu.
 *  - Single selection, single column as of now.
 * @return Idx of the selected menu item.
 * @retval MENU_NOT_SELECTED if no item was selected.
 */
int menu_set_get_user_choice(struct menu_set *mset);

#endif // TUI_H
