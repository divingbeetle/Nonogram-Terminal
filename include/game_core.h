#ifndef GAME_CORE_H
#define GAME_CORE_H

#include "puzzle.h"

struct cell 
{
    int row, col;
};

enum cell_state 
{
    CELL_EMPTY,
    CELL_FILLED,
    CELL_XMARKED
};

enum axis 
{
    AXIS_ROW,
    AXIS_COL
};

struct game_state 
{
    const struct puzzle *puzzle;
    enum cell_state **board_state;
};

struct game_state *game_state_create(const struct puzzle *pz);
void game_state_destroy(struct game_state *state);

#endif // GAME_CORE_H
