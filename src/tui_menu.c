#include "tui_menu.h"
#include "tui.h"
#include "utils.h"
#include <string.h>

#define MENU_CURSOR_MARK     " > "
#define MENU_CURSOR_MARK_LEN 3

struct hook_node
{
    Menu_Hook         func;
    struct hook_node *next;
};

struct menu_data
{
    struct hook_node  *menu_init_hook_list;
    struct hook_node  *menu_term_hook_list;
    struct hook_node  *item_init_hook_list;
    struct hook_node  *item_term_hook_list;
    const char        *title;
    struct directional padding;
    void              *any; // For user defined data
};

/* Function Prototypes */

struct menu_data *menu_data_init(void);
struct menu_data *menu_get_data(MENU *menu);
void              menu_data_destroy(struct menu_data *data);

ITEM **menu_items_create(char **choices, char **descriptions, int n_choices);

void adjust_menu_size(MENU *menu);

void add_hook(struct hook_node **head_indir, Menu_Hook hook_func);
void run_hook_list(struct hook_node *head, MENU *menu);
void free_hook_list(struct hook_node *head);

void run_menu_init_hook_list(MENU *menu);
void run_menu_term_hook_list(MENU *menu);
void run_item_init_hook_list(MENU *menu);
void run_item_term_hook_list(MENU *menu);

void menu_hook_show_title(MENU *menu);
void menu_hook_show_border(MENU *menu);
void menu_hook_show_description(MENU *menu);
void menu_hook_clean_menu(MENU *menu);

/* Public */

MENU *menu_create(char **choices, char **descriptions, int n_choices)
{
    assert(choices != NULL);

    ITEM **menu_items = menu_items_create(choices, descriptions, n_choices);
    ALLOC_CHECK_EXIT(menu_items);

    MENU *menu = new_menu(menu_items);
    ALLOC_CHECK_EXIT(menu);

    struct menu_data *data = menu_data_init();
    set_menu_userptr(menu, data);

    // Hooks
    set_menu_init(menu, run_menu_init_hook_list);
    set_menu_term(menu, run_menu_term_hook_list);
    set_item_init(menu, run_item_init_hook_list);
    set_item_term(menu, run_item_term_hook_list);

    // Options
    menu_opts_off(menu, O_SHOWDESC);  // Disable default description display use manual hook
    set_menu_mark(menu, MENU_CURSOR_MARK);
    set_menu_fore(menu, COLOR_PAIR(COLOR_P_YELLOW));

    // Add hook to show description when item is selected
    add_hook(&data->item_init_hook_list, menu_hook_show_description);

    WINDOW *win = newwin(0, 0, 0, 0);
    ALLOC_CHECK_EXIT(win);

    keypad(win, true);
    set_menu_win(menu, win);

    struct pos menu_size = {.y = menu->nitems, .x = menu->itemlen};
    wresize(win, menu_size.y, menu_size.x);
    set_menu_sub(menu, derwin(win, menu_size.y, menu_size.x, 0, 0));

    return menu;
}

void menu_destroy(MENU *menu)
{
    assert(menu != NULL);
    menu_data_destroy(menu_userptr(menu));

    if (menu_win(menu) != NULL)
    {
        werase(menu_win(menu));
        wrefresh(menu_win(menu));
        delwin(menu_win(menu));
    }

    free_menu(menu);
    menu = NULL;
}

void menu_set_main_window(MENU *menu, WINDOW *win)
{
    assert(menu != NULL);
    assert(win != NULL);

    set_menu_win(menu, win);
    adjust_menu_size(menu);

    // Show description for the initially selected item
    menu_hook_show_description(menu);
}

void menu_set_padding(MENU *menu, struct directional pad)
{
    assert(menu != NULL);

    struct menu_data *data = menu_get_data(menu);
    data->padding          = pad;
    adjust_menu_size(menu);
}

void menu_add_padding(MENU *menu, struct directional pad)
{
    assert(menu != NULL);

    struct menu_data *data = menu_get_data(menu);
    data->padding.top += pad.top;
    data->padding.bottom += pad.bottom;
    data->padding.left += pad.left;
    data->padding.right += pad.right;
    adjust_menu_size(menu);
}

int menu_get_user_input(MENU *menu)
{
    int ch = wgetch(menu_win(menu));

    switch (ch)
    {
        case KEY_DOWN:
        case 'j':
            menu_driver(menu, REQ_DOWN_ITEM);
            menu_hook_show_description(menu);
            break;
        case KEY_UP:
        case 'k':
            menu_driver(menu, REQ_UP_ITEM);
            menu_hook_show_description(menu);
            break;
        default:
            break;
    }

    return ch;
}

int menu_get_user_choice(MENU *menu)
{
    int choice = MENU_NOT_SELECTED;
    int ch;

    while ((ch = menu_get_user_input(menu)) != 'q')
    {
        switch (ch)
        {
            case KEY_ENTER:
            case 10:
                choice = item_index(current_item(menu));
                return choice;
                break;
        }
    }
    return choice;
}

void menu_set_title(MENU *menu, const char *title)
{
    struct menu_data *data = menu_get_data(menu);
    data->title            = title;

    struct directional title_pad = {
        .top = 2, .bottom = 0, .left = 0, .right = 0};
    menu_add_padding(menu, title_pad);
    add_hook(&data->menu_init_hook_list, menu_hook_show_title);
}

void menu_set_box(MENU *menu)
{
    struct menu_data *data = menu_get_data(menu);

    struct directional box_pad = {
        .top    = 1,
        .bottom = 3, // space for separation line + description + border
        .left   = 1,
        .right  = 3 // Extra space for balance with selection mark + description
    };
    menu_add_padding(menu, box_pad);
    add_hook(&data->menu_init_hook_list, menu_hook_show_border);
    add_hook(&data->item_init_hook_list, menu_hook_show_description);
}

/* Private */

ITEM **menu_items_create(char **choices, char **descriptions, int n_choices)
{
    assert(choices != NULL);

    // Ncurses menu requires NULL terminated array
    ITEM **items = calloc(n_choices + 1, sizeof(ITEM *));
    ALLOC_CHECK_EXIT(items);
    items[n_choices] = NULL;

    for (int i = 0; i < n_choices; i++)
    {
        char *desc = (descriptions == NULL) ? "" : descriptions[i];
        items[i]   = new_item(choices[i], desc);
        ALLOC_CHECK_EXIT(items[i]);
    }
    return items;
}

struct menu_data *menu_get_data(MENU *menu)
{
    return (struct menu_data *) menu_userptr(menu);
}

struct menu_data *menu_data_init(void)
{
    struct menu_data *data = malloc(sizeof(*data));
    ALLOC_CHECK_EXIT(data);
    data->title               = NULL;
    data->padding             = (struct directional) {0, 0, 0, 0};
    data->menu_init_hook_list = NULL;
    data->menu_term_hook_list = NULL;
    data->item_init_hook_list = NULL;
    data->item_term_hook_list = NULL;
    data->any                 = NULL;

    // Default hooks

    add_hook(&data->menu_term_hook_list, menu_hook_clean_menu);
    return data;
}

void add_hook(struct hook_node **head_indir, Menu_Hook hook_func)
{
    struct hook_node *new_node = malloc(sizeof(*new_node));
    ALLOC_CHECK_EXIT(new_node);
    new_node->func = hook_func;
    new_node->next = *head_indir;
    *head_indir    = new_node;
}

void run_hook_list(struct hook_node *head, MENU *menu)
{
    struct hook_node *curr = head;
    while (curr != NULL)
    {
        curr->func(menu);
        curr = curr->next;
    }
}

void free_hook_list(struct hook_node *head)
{
    struct hook_node *curr, *next;
    curr = head;
    while (curr != NULL)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }
    head = NULL;
}

void menu_data_destroy(struct menu_data *data)
{
    if (data != NULL)
    {
        free_hook_list(data->menu_init_hook_list);
        free_hook_list(data->menu_term_hook_list);
        free_hook_list(data->item_init_hook_list);
        free_hook_list(data->item_term_hook_list);
    }
    free(data);
    data = NULL;
}

void menu_hook_show_title(MENU *menu)
{
    WINDOW *win = menu_win(menu);

    struct menu_data *data  = (struct menu_data *) menu_userptr(menu);
    const char       *title = data->title;
    mvwprintw(win, 1, 2, "%s", title);
    mvwhline(win, 2, 1, ACS_HLINE, getmaxx(win) - 2);
}

void menu_hook_show_border(MENU *menu)
{
    WINDOW *win = menu_win(menu);
    box(win, 0, 0);
}

void adjust_menu_size(MENU *menu)
{
    struct directional padding  = menu_get_data(menu)->padding;
    struct pos         req_size = POS_ZERO;

    req_size.x =
        padding.left + MAX(menu->itemlen, menu->desclen) + padding.right;
    req_size.y = padding.top + menu->nitems + padding.bottom;

    struct pos win_size = get_window_size(menu_win(menu));

    if (win_size.y < req_size.y || win_size.x < req_size.x)
    {
        wresize(menu_win(menu), req_size.y, req_size.x);
    }
    mvderwin(menu_sub(menu), padding.top, padding.left);
}

void run_menu_init_hook_list(MENU *menu)
{
    run_hook_list(menu_get_data(menu)->menu_init_hook_list, menu);
}
void run_menu_term_hook_list(MENU *menu)
{
    run_hook_list(menu_get_data(menu)->menu_term_hook_list, menu);
}
void run_item_init_hook_list(MENU *menu)
{
    run_hook_list(menu_get_data(menu)->item_init_hook_list, menu);
}
void run_item_term_hook_list(MENU *menu)
{
    run_hook_list(menu_get_data(menu)->item_term_hook_list, menu);
}

void menu_hook_show_description(MENU *menu)
{
    WINDOW *win     = menu_win(menu);
    ITEM   *current = current_item(menu);

    int win_height = getmaxy(win);
    int win_width  = getmaxx(win);
    int status_y   = win_height - 2;

    // Clear status bar
    mvwhline(win, status_y, 1, ' ', win_width - 2);

    if (current && item_description(current))
    {
        // separator line
        mvwhline(win, status_y - 1, 1, ACS_HLINE, win_width - 2);

        wattron(win, COLOR_PAIR(COLOR_P_YELLOW) | A_DIM);
        mvwprintw(win, status_y, 2, "%s", item_description(current));
        wattroff(win, COLOR_PAIR(COLOR_P_YELLOW) | A_DIM);
    }

    wrefresh(win);
}
void menu_hook_clean_menu(MENU *menu)
{
    WINDOW *win = menu_win(menu);
    werase(win);
    wrefresh(win);
}
