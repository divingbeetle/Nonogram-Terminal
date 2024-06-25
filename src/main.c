#include <stdlib.h>
#include "cJSON/cJSON.h"
#include "game_control.h"
#include "loader.h"
#include "puzzle.h"
#include "tui.h"
#include "utils.h"

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

char *main_menu_title = "Main Menu";

int main(void)
{
    log_init();
    init_screen();

    struct menu_param params = 
    {
        .title        = main_menu_title,
        .size         = {.x = 40, .y = 10},
        .start        = {.x = 1,  .y = 1 },
        .n_choices    = MAIN_MENU_N_CHOICES,
        .choices      = main_menu_choices,
        .descriptions = NULL,
    };

    struct menu_set *mset = menu_set_create(&params);
    ALLOC_CHECK_EXIT(mset);

    struct menu_config config = menu_config_default;
    menu_set_configure(mset, config);

    bool in_menu = true;
    while (in_menu)
    {
        int menu_choice = menu_set_get_user_choice(mset);
        switch (menu_choice)
        {
            case MAIN_MENU_NEW_GAME:
                play(puzzle_get_user_choice(puzzle_set_get_user_choice()));
                break;

            case MAIN_MENU_CONTINUE:
                load_and_play();
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

    menu_set_destroy(mset);
    display_notification("Exiting...");
}
