#ifndef GAME_CORE_H
#define GAME_CORE_H

/******************************************************************************
 * CORE GAMEPLAY LOGIC
 *****************************************************************************/

#include "puzzle.h"
#include <stdbool.h>

enum cell_state 
{
    CELL_EMPTY,
    CELL_FILLED,
    CELL_XMARKED,
    CELL_TEMP_FILLED,
    CELL_TEMP_XMARKED
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
    enum cell_state **board_capture;
    struct undo_queue *undo_queue;
};

struct game_state *game_state_create(const struct puzzle *pz);
void game_state_destroy(struct game_state *state);

enum cell_state get_cell_state(const struct game_state *gs, struct cell cell);
void set_cell_state(struct game_state *gs, 
                    struct cell cell, enum cell_state state);

void game_state_save(const struct game_state *gs);
struct game_state *game_state_load(void);

/**
 * Toggles the cell between given state and empty state.
 */
void toggle_cell_state(struct game_state *gs, struct cell cell, enum cell_state new_state);

void set_area_state(struct game_state *gs, 
                    struct cell start, struct cell end,
                    enum cell_state new_state);

void set_area_if_empty(struct game_state *gs, 
                       struct cell start, struct cell end, 
                       enum cell_state new_state);

void switch_case(struct game_state *gs, struct cell start, struct cell end);
void clear_board(struct game_state *gs);

void auto_xmark(struct game_state *gs);
void delete_temp_marks(struct game_state *gs);
void store_capture(struct game_state *gs);
void restore_capture(struct game_state *gs);

void undo(struct game_state *gs);
void redo(struct game_state *gs);

bool validate_axis(const struct game_state *gs, enum axis axis, int idx);
bool game_solved(const struct game_state *gs);

#endif // GAME_CORE_H
