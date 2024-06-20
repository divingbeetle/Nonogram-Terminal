#include "game_control.h"

#include "game_core.h"
#include "game_ui.h"
#include "utils.h"

struct game_controller
{
    const struct puzzle *puzzle;
    struct game_state *state;
    struct game_ui *ui;
    struct cell cursor;
};

/* Function prototypes */
struct game_controller *game_controller_create(const struct puzzle *pz);
void game_controller_destroy(struct game_controller *game);

int run_game_loop(struct game_controller *game);

/* Public */

int play(const struct puzzle *pz)
{
    if (pz == NULL)
    {
        return -1;
    }

    struct game_controller *game = game_controller_create(pz);
    ALLOC_CHECK_RETURN(game, -1);
    display_base_board(game->ui);
    wgetch(game->ui->win);

    return 0;
}

/* Private */
struct game_controller *game_controller_create(const struct puzzle *pz)
{
    struct game_controller *game = malloc(sizeof(struct game_controller));
    ALLOC_CHECK_RETURN(game, NULL);

    struct game_state *gs = game_state_create(pz);
    ALLOC_CHECK_RETURN(gs, NULL);
    struct game_ui *ui = game_ui_create(pz);
    ALLOC_CHECK_RETURN(ui, NULL);

    game->puzzle = pz;
    game->state = gs;
    game->ui = ui;
    game->cursor = (struct cell){0, 0};

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

    for (;;)
    {

    }

    return 0;
}
