#ifndef GAME_UI_H
#define GAME_UI_H

#include "tui.h"
#include "puzzle.h"

struct game_ui 
{
    WINDOW *win;
    WINDOW *board;
    const struct puzzle *puzzle;
};

struct game_ui *game_ui_create(const struct puzzle *pz);
void game_ui_destroy(struct game_ui *ui);

void display_base_board(struct game_ui *ui);

#endif // GAME_UI_H
