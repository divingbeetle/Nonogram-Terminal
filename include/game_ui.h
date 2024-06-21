#ifndef GAME_UI_H
#define GAME_UI_H

/******************************************************************************
 * GAME_UI 
 *
 *****************************************************************************/

#include "tui.h"
#include "puzzle.h"
#include "game_core.h"

struct game_ui 
{
    WINDOW *win;
    WINDOW *board;
    const struct puzzle *puzzle;
};

struct game_ui *game_ui_create(const struct puzzle *pz);
void game_ui_destroy(struct game_ui *ui);

void highlight_area(struct game_ui *ui, struct cell start, struct cell end, attr_t attr);

/**
 * Display the base puzzle board with row and column clues.
 */
void display_base_board(struct game_ui *ui);

/**
 * Update the ui with the current game state 
 */
void display_game_state(struct game_ui *ui, const struct game_state *state);

#endif // GAME_UI_H
