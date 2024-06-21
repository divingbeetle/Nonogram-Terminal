#include "tui.h"
#include "utils.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define KEY_LF 10

const struct menu_config menu_config_default =
{
    .rescale = true,
};

/* Function Prototypes */

void end_screen(void);
ITEM **menu_items_create(const struct menu_param *params);

bool enough_screen_size(struct pos required_size)
{
    struct pos screen_size;
    getmaxyx(stdscr, screen_size.y, screen_size.x);
    return (screen_size.y >= required_size.y && screen_size.x >= required_size.x);
}

/* Public */

void init_screen(void)
{
    initscr();
    atexit(end_screen);

    cbreak();
    noecho();
    keypad(stdscr, true);
    curs_set(0);
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

struct menu_set *menu_set_create(const struct menu_param *params)
{
    assert(params != NULL);

    struct menu_set *mset = malloc(sizeof(struct menu_set));
    ALLOC_CHECK_RETURN(mset, NULL);

    ITEM **menu_items = menu_items_create(params);
    ALLOC_CHECK_RETURN(menu_items, NULL);

    MENU *menu = new_menu(menu_items);
    ALLOC_CHECK_RETURN(menu, NULL);

    WINDOW *win = newwin(params->size.y, params->size.x, 
                         params->start.y, params->start.x);
    ALLOC_CHECK_RETURN(win, NULL);
    mset->params = params;
    mset->win    = win;
    mset->menu   = menu;
    mset->items  = menu_items;
    set_menu_win(menu, win);

    return mset;
}

void menu_set_configure(struct menu_set *mset, const struct menu_config config)
{
    assert(mset != NULL);

    keypad(mset->win, true);
    // @TODO
    // set_menu_format();
    // set_menu_fore();
    // set_menu_back();
    set_menu_mark(mset->menu, " > ");

    struct pos req_size;
    scale_menu(mset->menu, &req_size.y, &req_size.x);
    if (config.rescale == true)
    {
        if (mset->params->size.y < req_size.y + 2
            || mset->params->size.x < req_size.x + 2)
        {
            wresize(mset->win, req_size.y + 2, req_size.x + 2);
        }
    }

    set_menu_sub(mset->menu, derwin(mset->win, req_size.y, req_size.x, 1, 1));
}

void menu_set_destroy(struct menu_set *mset)
{
    assert(mset != NULL);
    assert(mset->menu != NULL);
    assert(mset->win != NULL);

    wclear(mset->win);
    wrefresh(mset->win);
    free_menu(mset->menu);
    for (int i = 0; i < mset->params->n_choices; i++)
    {
        free_item(mset->items[i]);
    }
    delwin(mset->win);
    free(mset->items);
    free(mset);
}

int menu_set_get_user_choice(struct menu_set *mset)
{
    assert(mset != NULL);
    assert(mset->menu != NULL);
    assert(mset->win != NULL);

    box(mset->win, 0, 0);
    post_menu(mset->menu);
    wrefresh(mset->win);
    int choice;
    while ((choice = wgetch(mset->win)) != 'q')
    {
        switch (choice)
        {
            case KEY_DOWN: case 'j':
                menu_driver(mset->menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP: case 'k':
                menu_driver(mset->menu, REQ_UP_ITEM);
                break;
            case KEY_ENTER: case KEY_LF:
                unpost_menu(mset->menu);
                return item_index(current_item(mset->menu));
                break;
        }
        wrefresh(mset->win);
    }
    unpost_menu(mset->menu);
    return MENU_NOT_SELECTED;
}

/* Private */

void end_screen(void) 
{ 
    endwin();
}

ITEM **menu_items_create(const struct menu_param *params)
{
    ITEM **items = calloc(params->n_choices + 1, sizeof(ITEM *));
    ALLOC_CHECK_RETURN(items, NULL);

    int i;
    for (i = 0; i < params->n_choices; i++)
    {
        char *desc = (params->descriptions == NULL)
                          ? "" 
                          : params->descriptions[i];

        items[i] = new_item(params->choices[i], desc);
        if (items[i] == NULL)
        {
            break;
        }
    }

    if (i != params->n_choices)
    {
        for (int j = 0; j < i; j++)
        {
            free_item(items[j]);
        }
        free(items);
        return NULL;
    }

    // Ncurses menu requires NULL terminated array
    items[params->n_choices] = (ITEM *) NULL;

    return items;
}
