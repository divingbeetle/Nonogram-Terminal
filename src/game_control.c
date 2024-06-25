#include "game_control.h"
#include "game_core.h"
#include "game_ui.h"
#include "tui.h"
#include "utils.h"

#define EXIT_GAME -1

enum controler_mode 
{
    MODE_NORMAL,
    MODE_VISUAL,
    MODE_COMMAND
};

struct game_controller
{
    const struct puzzle *puzzle;
    struct game_state *state;
    struct game_ui *ui;
    enum controler_mode mode;
    struct cell cursor;
    struct cell selection_pivot; // For visual mode selection area
};

char *command_mode_choices[CMD_N] =
{
    [CMD_AUTO_XMARK]        = "Auto XMark",
    [CMD_DELETE_TEMP_MARKS] = "Delete Temp Marks",
    [CMD_CLEAR]             = "Clear",
    [CMD_CAPTURE]           = "Capture",
    [CMD_RESTORE_CAPTURE]   = "Restore Capture",
    [CMD_SAVE]              = "Save",
    [CMD_QUIT]              = "Quit",
};

char *command_mode_desc[CMD_N] = 
{
    [CMD_AUTO_XMARK]        = "X mark all cells on correct axis",
    [CMD_DELETE_TEMP_MARKS] = "Delete all temporary marks",
    [CMD_CLEAR]             = "Clear the board",
    [CMD_CAPTURE]           = "Capture current state",
    [CMD_RESTORE_CAPTURE]   = "Restore captured state",
    [CMD_SAVE]              = "Save the current state",
    [CMD_QUIT]              = "Quit the game",
};


/* Function prototypes */

struct game_controller *game_controller_create(const struct puzzle *pz);
void game_controller_destroy(struct game_controller *game);

int run_game_loop(struct game_controller *game);

int handle_key_input(struct game_controller *game, int key);
bool handle_cursor_movement(struct game_controller *game, int key);
bool handle_edit(struct game_controller *game, int key);
bool handle_operation(struct game_controller *game, int key);
bool handle_mode_switch(struct game_controller *game, int key);

void toggle_visual_mode(struct game_controller *game);

void clamp_cursor(struct game_controller *game);

struct cell selection_start(struct game_controller *game);
struct cell selection_end(struct game_controller *game);

int open_command_mode(struct game_controller *game);

/* Public */

int play(const struct puzzle *pz)
{
    assert(pz != NULL);

    struct game_controller *game = game_controller_create(pz);
    display_base_board(game->ui);
    run_game_loop(game);

    return 0;
}

int load_and_play(void)
{
    struct game_state *gs = game_state_load();
    if (gs == NULL)
    {
        return -1;
    }

    struct game_controller *game = malloc(sizeof(*game));
    ALLOC_CHECK_EXIT(game);

    struct game_ui *ui    = game_ui_create(gs->puzzle);

    game->puzzle = gs->puzzle;
    game->state  = gs;
    game->ui     = ui;
    game->cursor = (struct cell){0, 0};
    game->mode   = MODE_NORMAL;

    display_base_board(game->ui);
    run_game_loop(game);

    return 0;
}

/* Private */

struct game_controller *game_controller_create(const struct puzzle *pz)
{
    struct game_controller *game = malloc(sizeof(*game));
    ALLOC_CHECK_EXIT(game);

    struct game_state *gs = game_state_create(pz);
    struct game_ui *ui    = game_ui_create(pz);

    game->puzzle = pz;
    game->state  = gs;
    game->ui     = ui;
    game->cursor = (struct cell){0, 0};
    game->mode   = MODE_NORMAL;

    return game;
}

void game_controller_destroy(struct game_controller *game)
{
    assert(game != NULL);
    assert(game->state != NULL);
    assert(game->ui != NULL);

    game_state_destroy(game->state);
    game_ui_destroy(game->ui);
    free(game);
}

int run_game_loop(struct game_controller *game)
{
    assert (game != NULL);

    game->cursor = (struct cell){0, 0};
    for (;;)
    {
        if (game->mode == MODE_NORMAL)
        {
            game->selection_pivot = game->cursor;
        }

        struct cell start = selection_start(game);
        struct cell end   = selection_end(game);

        highlight_area(game->ui, start, end, A_REVERSE);
        display_game_state(game->ui, game->state);
        int key = wgetch(game->ui->win);
        highlight_area(game->ui, start, end, A_NORMAL);

        if (handle_key_input(game, key) == EXIT_GAME)
        {
            return 0;
        }

        if (game_solved(game->state))
        {
            // @TODO: Make cool display
            display_notification("Goodjob");
            return 0;
        }
    }

    return 0;
}

bool handle_cursor_movement(struct game_controller *game, int key)
{
    switch (key)
    {
        // Basic movements
        case KEY_UP: case 'k': case 'K':
            game->cursor.row--;
            break;
        case KEY_DOWN: case 'j': case 'J':
            game->cursor.row++;
            break;
        case KEY_LEFT: case 'h': case 'H':
            game->cursor.col--;
            break;
        case KEY_RIGHT: case 'l': case 'L':
            game->cursor.col++;
            break;

        // Jump movements
        case 'I': case '0':
            game->cursor.col = 0;
            break;
        case 'A': case '$':
            game->cursor.col = game->puzzle->n_cols - 1;
            break;
        case 'g':
            game->cursor.row = 0;
            break;
        case 'G':
            game->cursor.row = game->puzzle->n_rows - 1;
            break;
        case 'w': case 'W':
            game->cursor.col += 5;
            break;
        case 'b': case 'B':
            game->cursor.col -= 5;
            break;
        case 'o': case '{':
            game->cursor.row += 5;
            break;
        case 'O': case '}':
            game->cursor.row -= 5;
            break;

        default:
            return false;
            break;
    }

    clamp_cursor(game);
    return true;
}

bool handle_edit(struct game_controller *game, int key)
{
    enum cell_state new_state;

    struct cell start = selection_start(game);
    struct cell end   = selection_end(game);
    switch (key)
    {
        case 'f': 
            new_state = CELL_FILLED;
            break;
        case 'x':
            new_state = CELL_XMARKED;
            break;
        // Shift + key for temprorary marks
        case 'F':
            new_state = CELL_TEMP_FILLED;
            break;
        case 'X':
            new_state = CELL_TEMP_XMARKED;
            break;

        case 'd':
            new_state = CELL_EMPTY;
            break;

        case '~':
            switch_case(game->state, start, end);
            return true;
            break;
        default:
            return false;
            break;
    }

    if (game->mode == MODE_NORMAL)
    {
        toggle_cell_state(game->state, game->cursor, new_state);
    }
    else if (game->mode == MODE_VISUAL)
    {
        if (new_state == CELL_EMPTY)
        {
            set_area_state(game->state, start, end, CELL_EMPTY);
        }
        else 
        {
            set_area_if_empty(game->state, start, end, new_state);
        }
        game->mode = MODE_NORMAL;
    }

    return true;
}

bool handle_operation(struct game_controller *game, int key)
{
    switch (key)
    {
        case 'u':
            undo(game->state);
            break;
        case 'U': 
            redo(game->state);
            break;
        case '?':
            // @TODO: help
            break;
        default:
            return false;
            break;
    }

    return true;
}

bool handle_mode_switch(struct game_controller *game, int key)
{
    switch (key)
    {
        case 'v':
            toggle_visual_mode(game);
            break;
        case 'V':
            game->mode = MODE_VISUAL;
            game->selection_pivot.row = game->cursor.row;
            game->selection_pivot.col = 0;
            game->cursor.col = game->puzzle->n_cols - 1;
            break;
        default:
            return false;
            break;
    }
    return true;
}


int handle_key_input(struct game_controller *game, int key)
{
    if (key == 'q')
        return EXIT_GAME;

    if (handle_cursor_movement(game, key))
        return 0;

    if (handle_edit(game, key))
        return 0;

    if (handle_operation(game, key))
        return 0;

    if (handle_mode_switch(game, key))
        return 0;

    if (key == ':')
    {
        return open_command_mode(game);
    }

    if (key == KEY_RESIZE)
    {
        // @TODO:
    }

    // Any Unhandled key will exit visual mode
    if (game->mode == MODE_VISUAL)
    {
        game->mode = MODE_NORMAL;
    }
    return 0;
}

void clamp_cursor(struct game_controller *game)
{
    game->cursor.row = MAX(0, MIN(game->cursor.row, game->puzzle->n_rows - 1));
    game->cursor.col = MAX(0, MIN(game->cursor.col, game->puzzle->n_cols - 1));
}

void toggle_visual_mode(struct game_controller *game)
{
    if (game->mode == MODE_VISUAL)
    {
        game->mode = MODE_NORMAL;
    }
    else
    {
        game->selection_pivot = game->cursor;
        game->mode = MODE_VISUAL;
    }
}

struct cell selection_start(struct game_controller *game)
{
    struct cell start = 
    {
        .row = MIN(game->selection_pivot.row, game->cursor.row),
        .col = MIN(game->selection_pivot.col, game->cursor.col)
    };

    return start;
}
struct cell selection_end(struct game_controller *game)
{
    struct cell end = 
    {
        .row = MAX(game->selection_pivot.row, game->cursor.row),
        .col = MAX(game->selection_pivot.col, game->cursor.col)
    };

    return end;
}

int open_command_mode(struct game_controller *game)
{
    int cmd_choice = menu_set_get_user_choice(game->ui->cmd_menu);
    wclear(game->ui->cmd_menu->win);
    wrefresh(game->ui->cmd_menu->win);

    switch (cmd_choice)
    {
        case CMD_AUTO_XMARK:
            auto_xmark(game->state);
            break;
        case CMD_DELETE_TEMP_MARKS:
            delete_temp_marks(game->state);
            break;
        case CMD_CLEAR:
            clear_board(game->state);
            break;
        case CMD_CAPTURE:
            store_capture(game->state);
            break;
        case CMD_RESTORE_CAPTURE:
            restore_capture(game->state);
            break;
        case CMD_SAVE:
            game_state_save(game->state);
            break;
        case CMD_QUIT:
            return EXIT_GAME;
            break;
        case MENU_NOT_SELECTED:
            break;
        default:
            LOGF(LOG_WARNING, "Invalid command choice: %d", cmd_choice);
            break;
    }
    return 0;
}
