#include "game_control.h"
#include "tui.h"
#include "tui_menu.h"
#include "utils.h"
#include <menu.h>

#define TITLE_TEXT_HEIGHT 20
#define TITLE_TEXT_WIDTH  88

void display_title_screen(void)
{
    const char *title_text[TITLE_TEXT_HEIGHT] = {
        "                                                                      "
        "      ",
        " /$$   /$$  /$$$$$$  /$$   /$$  /$$$$$$   /$$$$$$  /$$$$$$$   /$$$$$$ "
        " /$$      /$$\n",
        "| $$$ | $$ /$$__  $$| $$$ | $$ /$$__  $$ /$$__  $$| $$__  $$ /$$__  "
        "$$| $$$    /$$$\n",
        "| $$$$| $$| $$  \\ $$| $$$$| $$| $$  \\ $$| $$  \\__/| $$  \\ $$| $$  "
        "\\ $$| $$$$  /$$$$\n",
        "| $$ $$ $$| $$  | $$| $$ $$ $$| $$  | $$| $$ /$$$$| $$$$$$$/| "
        "$$$$$$$$| $$ $$/$$ $$\n",
        "| $$  $$$$| $$  | $$| $$  $$$$| $$  | $$| $$|_  $$| $$__  $$| $$__  "
        "$$| $$  $$$| $$\n",
        "| $$\\  $$$| $$  | $$| $$\\  $$$| $$  | $$| $$  \\ $$| $$  \\ $$| $$  "
        "| $$| $$\\  $ | $$\n",
        "| $$ \\  $$|  $$$$$$/| $$ \\  $$|  $$$$$$/|  $$$$$$/| $$  | $$| $$  | "
        "$$| $$ \\/  | $$\n",
        "|__/  \\__/ \\______/ |__/  \\__/ \\______/  \\______/ |__/  |__/|__/ "
        " |__/|__/     |__/\n",
        "                                                                      "
        "      \n",
        "                                                                      "
        "      \n",
        "   /$$$$$$$$ /$$$$$$$$ /$$$$$$$  /$$      /$$ /$$$$$$ /$$   /$$  "
        "/$$$$$$  /$$\n",
        "  |__  $$__/| $$_____/| $$__  $$| $$$    /$$$|_  $$_/| $$$ | $$ /$$__ "
        " "
        "$$| $$\n",
        "     | $$   | $$      | $$  \\ $$| $$$$  /$$$$  | $$  | $$$$| $$| $$  "
        "\\ $$| $$\n",
        "     | $$   | $$$$$   | $$$$$$$/| $$ $$/$$ $$  | $$  | $$ $$ $$| "
        "$$$$$$$$| $$\n",
        "     | $$   | $$__/   | $$__  $$| $$  $$$| $$  | $$  | $$  $$$$| $$__ "
        " "
        "$$| $$\n",
        "     | $$   | $$      | $$  \\ $$| $$\\  $ | $$  | $$  | $$\\  $$$| "
        "$$ "
        " | $$| $$\n",
        "     | $$   | $$$$$$$$| $$  | $$| $$ \\/  | $$ /$$$$$$| $$ \\  $$| $$ "
        " "
        "| $$| $$$$$$$$\n",
        "     |__/   |________/|__/  |__/|__/     |__/|______/|__/  \\__/|__/  "
        "|__/|________/\n",
        "                                                                      "
        "     \n",
    };

    int screen_width = getmaxx(stdscr);

    int title_y = 1;
    int title_x = (screen_width - TITLE_TEXT_WIDTH) / 2;

    attron(COLOR_PAIR(COLOR_P_CYAN) | A_BLINK);
    for (int i = 0; i < TITLE_TEXT_HEIGHT; i++)
    {
        mvprintw(title_y + i, title_x, "%s", title_text[i]);
    }
    attroff(COLOR_PAIR(COLOR_P_CYAN) | A_BLINK);
    refresh();
}

enum main_menu
{
    MAIN_MENU_NEW_GAME,
    MAIN_MENU_CONTINUE,
    MAIN_MENU_HOW_TO_PLAY,
    MAIN_MENU_SETTINGS,
    MAIN_MENU_EXIT,
    MAIN_MENU_DEBUG,
    MAIN_MENU_N_CHOICES
};

char *main_menu_choices[MAIN_MENU_N_CHOICES] = 
{
    [MAIN_MENU_NEW_GAME]    = "New Game",
    [MAIN_MENU_CONTINUE]    = "Continue",
    [MAIN_MENU_HOW_TO_PLAY] = "How to Play",
    [MAIN_MENU_SETTINGS]    = "Settings",
    [MAIN_MENU_EXIT]        = "Exit",
    [MAIN_MENU_DEBUG]       = "DEBUG"
};

char *main_menu_descriptions[MAIN_MENU_N_CHOICES] = 
{
    [MAIN_MENU_NEW_GAME]    = "Start a new game",
    [MAIN_MENU_CONTINUE]    = "Continue from save file",
    [MAIN_MENU_HOW_TO_PLAY] = "Learn how to play",
    [MAIN_MENU_SETTINGS]    = "Change game settings",
    [MAIN_MENU_EXIT]        = "Exit game",
    [MAIN_MENU_DEBUG]       = "Debug "
};

char *main_menu_title = "Main Menu";

int main(void)
{
    log_init();
    init_screen();

    display_title_screen();
    refresh();
    getch();
    clear();

    MENU *main_menu = menu_create(
        main_menu_choices, main_menu_descriptions, MAIN_MENU_N_CHOICES);
    menu_set_box(main_menu);
    menu_set_title(main_menu, main_menu_title);

    int screen_width = getmaxx(stdscr);
    int screen_height = getmaxy(stdscr);

    WINDOW *menu_window = menu_win(main_menu);
    int menu_width = getmaxx(menu_window);
    int menu_height = getmaxy(menu_window);
    
    int menu_x = (screen_width - menu_width) / 2;
    int menu_y = TITLE_TEXT_HEIGHT + 3; // Add some space between title and menu
    
    if (menu_y + menu_height > screen_height) {
        menu_y = screen_height - menu_height - 1;
    }
    
    mvwin(menu_window, menu_y, menu_x);

    bool in_menu = true;
    while (in_menu)
    {
        post_menu(main_menu);
        int menu_choice = menu_get_user_choice(main_menu);
        unpost_menu(main_menu);
        switch (menu_choice)
        {
            case MAIN_MENU_NEW_GAME:
                new_game();
                break;

            case MAIN_MENU_CONTINUE:
                continue_game();
                break;

            case MAIN_MENU_HOW_TO_PLAY:
                display_notification("Unimplemented");
                break;

            case MAIN_MENU_SETTINGS:
                display_notification("Unimplemented");
                break;

            case MAIN_MENU_EXIT:
                in_menu = false;
                break;

            case MAIN_MENU_DEBUG:
                color_pairs_test();
                refresh();
                getch();
                clear();

                color_test();
                refresh();
                getch();
                clear();
                break;

            case MENU_NOT_SELECTED:
                LOG(LOG_INFO, "Menu not selected");
                display_notification("No choice selected");
                in_menu = false;
                break;

            default:
                LOGF(LOG_ERROR, "Invalid choice: %d", menu_choice);
                display_notification("Invalid choice");
                in_menu = false;
                break;
        }
    }
    menu_destroy(main_menu);
    display_notification("Exiting...");
}
