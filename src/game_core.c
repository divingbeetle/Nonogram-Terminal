#include "game_core.h"
#include "utils.h"

/* Function prototypes */ 

/* Public */
struct game_state *game_state_create(const struct puzzle *pz)
{
    assert(pz != NULL);

    struct game_state *gs = malloc(sizeof(struct game_state));
    ALLOC_CHECK_RETURN(gs, NULL);
    
    enum cell_state **board_state = (enum cell_state **)alloc2d(pz->n_rows, pz->n_cols, sizeof(enum cell_state));
    ALLOC_CHECK_RETURN(board_state, NULL);
    for (int i = 0; i < pz->n_rows; i++)
    {
        for (int j = 0; j < pz->n_cols; j++)
        {
            board_state[i][j] = CELL_EMPTY;
        }
    }

    gs->puzzle = pz;
    gs->board_state = board_state;

    return gs;
}

void game_state_destroy(struct game_state *gs)
{
    assert(gs != NULL);
    assert(gs->board_state != NULL);
    assert(gs->puzzle != NULL);

    free2d((void **)gs->board_state, gs->puzzle->n_rows);
    free(gs);
}

/* Private */


