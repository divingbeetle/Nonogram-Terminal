#ifndef TUI_MENU_H
#define TUI_MENU_H

/*******************************************************************************
 * TUI_MENU
 *
 * Ncuses menu wrapper
 ******************************************************************************/

#include "tui.h"
#include <menu.h>

#define MENU_NOT_SELECTED -1

/**
 * @brief  Create ncurses menu with predefined data structure embedded
 * @see    struct menu_data for more information
 *
 * Menu windows (menu_win and menu_sub) are created and sized automatically
 * based on provided choices and descriptions.
 */
MENU *menu_create(char **choices, char **descriptions, int n_choices);

/**
 * @brief  Destroy menu and free all resources
 */
void menu_destroy(MENU *menu);

void menu_set_main_window(MENU *menu, WINDOW *win);
int  menu_get_user_choice(MENU *menu);

/**
 * @brief  Set padding between menu items and menu window
 * @note   This overrides paddings set by other functions
 */
void menu_set_padding(MENU *menu, struct directional pad);
void menu_add_padding(MENU *menu, struct directional pad);

void menu_set_box(MENU *menu);
void menu_set_title(MENU *menu, const char *title);

int menu_get_user_input(MENU *menu);
int menu_get_user_choice(MENU *menu);

#endif // TUI_MENU_H
