#include "game_core.h"
#include "config.h"
#include "utils.h"
#include <string.h>

#define UNDO_LIMIT 1024 // Should be power of 2
#define UNDO_NEXT_INDEX(i) (((i) + 1) & (UNDO_LIMIT - 1))
#define UNDO_PREV_INDEX(i) (((i) - 1) & (UNDO_LIMIT - 1))

#define UNDO_ENTRY_UNMODIFIED 0

struct undo_entry 
{
    struct cell cell;
    enum cell_state state;
    int n_modified;
};

struct undo_queue 
{
    int head, tail;
    struct undo_entry data[UNDO_LIMIT];
}; 


/* Function prototypes */ 

struct undo_queue *undo_queue_create(void);
void undo_queue_push(struct undo_queue *q, struct undo_entry entry);

/**
 * Set cell's state to new_state, push undo entry if modified
 *  - Multiple function calls with same modified_cnt addr will be backtracked 
 *    with a single undo/redo operation
 *
 * @param modified_cnt addr of modified counter
 *  - Must be Initialized with UNDO_ENTRY_UNMODIFIED
 *  - NULL if undo entry is not needed
 */
void set_cell_state_internal(struct game_state *gs, 
                             struct cell curr, enum cell_state new_state,
                             int *modified_cnt);

enum cell_state **board_state_create(const struct puzzle *pz);

/* Public */

struct game_state *game_state_create(const struct puzzle *pz)
{
    assert(pz != NULL);

    struct game_state *gs = malloc(sizeof(struct game_state));
    ALLOC_CHECK_EXIT(gs);

    gs->puzzle      = pz;
    gs->board_state = board_state_create(pz);
    gs->undo_queue  = undo_queue_create();
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

void set_cell_state(struct game_state *gs, 
                    struct cell cell, enum cell_state new_state)
{
    int modified_cnt = UNDO_ENTRY_UNMODIFIED;
    set_cell_state_internal(gs, cell, new_state, &modified_cnt);
}

enum cell_state get_cell_state(const struct game_state *gs, struct cell cell)
{
    assert(gs != NULL);
    assert(gs->board_state != NULL);
    assert(cell.row >= 0 && cell.row < gs->puzzle->n_rows);
    assert(cell.col >= 0 && cell.col < gs->puzzle->n_cols);

    return gs->board_state[cell.row][cell.col];
}

void toggle_cell_state(struct game_state *gs, 
                       struct cell cell, enum cell_state new_state)
{
    new_state = get_cell_state(gs, cell) == new_state ? CELL_EMPTY : new_state;
    set_cell_state(gs, cell, new_state);
}

void set_area_state(struct game_state *gs, 
                    struct cell start, struct cell end, 
                    enum cell_state new_state)
{
    int modified_cnt = UNDO_ENTRY_UNMODIFIED;
    struct cell curr;
    for (curr.row = start.row; curr.row <= end.row; curr.row++)
    {
        for (curr.col = start.col; curr.col <= end.col; curr.col++)
        {
            set_cell_state_internal(gs, curr, new_state, &modified_cnt);
        }
    }
}

void set_area_if_empty(struct game_state *gs, 
                       struct cell start, struct cell end, 
                       enum cell_state new_state)
{
    int modified_cnt = UNDO_ENTRY_UNMODIFIED;
    struct cell curr;
    for (curr.row = start.row; curr.row <= end.row; curr.row++)
    {
        for (curr.col = start.col; curr.col <= end.col; curr.col++)
        {
            if (get_cell_state(gs, curr) == CELL_EMPTY)
            {
                set_cell_state_internal(gs, curr, new_state, &modified_cnt);
            }
        }
    }
}

void delete_temp_marks(struct game_state *gs)
{
    struct cell curr;
    int modified_cnt = UNDO_ENTRY_UNMODIFIED;
    for (curr.row = 0; curr.row < gs->puzzle->n_rows; curr.row++)
    {
        for (curr.col = 0; curr.col < gs->puzzle->n_cols; curr.col++)
        {
            if (get_cell_state(gs, curr) == CELL_TEMP_FILLED
                || get_cell_state(gs, curr) == CELL_TEMP_XMARKED)
            {
                set_cell_state_internal(gs, curr, CELL_EMPTY, &modified_cnt);
            }
        }
    }
}

void switch_case(struct game_state *gs, struct cell start, struct cell end)
{
    struct cell curr;
    int modified_cnt = UNDO_ENTRY_UNMODIFIED;
    for (curr.row = start.row; curr.row <= end.row; curr.row++)
    {
        for (curr.col = start.col; curr.col <= end.col; curr.col++)
        {
            enum cell_state new_state;
            switch (get_cell_state(gs, curr))
            {
                case CELL_FILLED:
                    new_state = CELL_TEMP_FILLED;
                    break;
                case CELL_XMARKED:
                    new_state = CELL_TEMP_XMARKED;
                    break;
                case CELL_TEMP_FILLED:
                    new_state = CELL_FILLED;
                    break;
                case CELL_TEMP_XMARKED:
                    new_state = CELL_XMARKED;
                    break;
                default:
                    continue;
            }
            set_cell_state_internal(gs, curr, new_state, &modified_cnt);
        }
    }
}

void clear_board(struct game_state *gs)
{
    struct cell start = {0, 0};
    struct cell end   = {gs->puzzle->n_rows - 1, gs->puzzle->n_cols - 1};
    set_area_state(gs, start, end, CELL_EMPTY);
}

void auto_xmark(struct game_state *gs)
{
    int n_modified = 0;
    struct cell curr;

    for (curr.row = 0; curr.row < gs->puzzle->n_rows; curr.row++)
    {
        if (validate_axis(gs, AXIS_ROW, curr.row))
        {
            for (curr.col = 0; curr.col < gs->puzzle->n_cols; curr.col++)
            {
                if (get_cell_state(gs, curr) == CELL_EMPTY)
                {
                    set_cell_state_internal(gs, curr, CELL_XMARKED, &n_modified);
                }
            }
        }
    }

    for (curr.col = 0; curr.col < gs->puzzle->n_cols; curr.col++)
    {
        if (validate_axis(gs, AXIS_COL, curr.col))
        {
            for (curr.row = 0; curr.row < gs->puzzle->n_rows; curr.row++)
            {
                if (get_cell_state(gs, curr) == CELL_EMPTY)
                {
                    set_cell_state_internal(gs, curr, CELL_XMARKED, &n_modified);
                }
            }
        }
    }
}

void store_capture(struct game_state *gs)
{
    assert(gs != NULL);
    if (gs->board_capture == NULL)
    {
        gs->board_capture = board_state_create(gs->puzzle);
    }

    for (int i = 0; i < gs->puzzle->n_rows; i++)
    {
        for (int j = 0; j < gs->puzzle->n_cols; j++)
        {
            gs->board_capture[i][j] = gs->board_state[i][j];
        }
    }
}

void restore_capture(struct game_state *gs)
{
    assert(gs != NULL);
    if (gs->board_capture == NULL)
    {
        return;
    }

    struct cell curr;
    int n_modified = UNDO_ENTRY_UNMODIFIED;

    for (curr.row = 0; curr.row < gs->puzzle->n_rows; curr.row++)
    {
        for (curr.col = 0; curr.col < gs->puzzle->n_cols; curr.col++)
        {
            set_cell_state_internal(gs, curr, 
                                    gs->board_capture[curr.row][curr.col], 
                                    &n_modified);
        }
    }

}

void undo(struct game_state *gs)
{
    assert(gs != NULL);
    assert(gs->undo_queue != NULL);

    struct undo_queue *q = gs->undo_queue;

    struct undo_entry curr_entry = q->data[UNDO_PREV_INDEX(q->tail)];
    int n_undo = curr_entry.n_modified;
    while (n_undo-- > 0 && q->head != q->tail)
    {
        q->tail = UNDO_PREV_INDEX(q->tail);
        curr_entry = q->data[q->tail];

        // Store current state for redo
        q->data[q->tail].state = get_cell_state(gs, curr_entry.cell);
        set_cell_state_internal(gs, curr_entry.cell, curr_entry.state, NULL);
    }
}

void redo(struct game_state *gs)
{
    assert(gs != NULL);
    assert(gs->undo_queue != NULL);

    struct undo_queue *q = gs->undo_queue;

    struct undo_entry next_entry = q->data[q->tail];
    int n_modified = 0;

    // Redo until n_modified stops increasing
    while (next_entry.n_modified > n_modified)
    {
        // Store current state for undo
        q->data[q->tail].state = get_cell_state(gs, next_entry.cell);
        set_cell_state_internal(gs, next_entry.cell, next_entry.state, NULL);

        n_modified = next_entry.n_modified;
        q->tail = UNDO_NEXT_INDEX(q->tail);
        next_entry = q->data[q->tail];

        if (q->tail == q->head)
        {
            break;
        }
    }
}

bool validate_axis(const struct game_state *gs, enum axis axis, int idx)
{
    int *clueline = axis == AXIS_ROW 
                    ? gs->puzzle->row_clues[idx] 
                    : gs->puzzle->col_clues[idx];

    int curr_clue_idx = axis == AXIS_ROW 
                        ? get_row_clueline_size(gs->puzzle) - 1
                        : get_col_clueline_size(gs->puzzle) - 1;
    struct cell curr = 
    {
        .row = (axis == AXIS_ROW) ? idx : gs->puzzle->n_rows - 1,
        .col = (axis == AXIS_ROW) ? gs->puzzle->n_cols - 1 : idx
    };

    int run_length;
    while (curr_clue_idx >= 0)
    {
        run_length = 0;
        while (curr.row >= 0 && curr.col >= 0
               && get_cell_state(gs, curr) != CELL_FILLED)
        {
            curr.row -= (axis == AXIS_COL);
            curr.col -= (axis == AXIS_ROW);
        }

        while (curr.row >= 0 && curr.col >= 0
               && get_cell_state(gs, curr) == CELL_FILLED)
        {
            run_length++;
            curr.row -= (axis == AXIS_COL);
            curr.col -= (axis == AXIS_ROW);
        }

        if (run_length != clueline[curr_clue_idx])
        {
            return false;
        }

        curr_clue_idx--;
    }

    return true;
}

bool game_solved(const struct game_state *gs)
{
    for (int i = 0; i < gs->puzzle->n_rows; i++)
    {
        if (!validate_axis(gs, AXIS_ROW, i))
        {
            return false;
        }
    }

    for (int j = 0; j < gs->puzzle->n_cols; j++)
    {
        if (!validate_axis(gs, AXIS_COL, j))
        {
            return false;
        }
    }

    return true;
}

void game_state_save(const struct game_state *gs)
{
    assert(gs != NULL);

    FILE *fp = fopen(SAVE_FILE_NAME, "wb");
    if (fp == NULL)
    {
        LOGF(LOG_WARNING, "Failed to open file: %s", SAVE_FILE_NAME);
        return;
    }

    // Puzzle 
    
    const struct puzzle *pz = gs->puzzle;

    fwrite(pz->title, sizeof(char), MAX_PZ_TITLE_LEN, fp);
    fwrite(pz->author, sizeof(char), MAX_PZ_AUTHOR_LEN, fp);
    fwrite(&pz->difficulty, sizeof(int), 1, fp);
    fwrite(&pz->n_rows, sizeof(int), 1, fp);
    fwrite(&pz->n_cols, sizeof(int), 1, fp);

    fwrite(pz->row_clues[0], sizeof(int), pz->n_rows * get_row_clueline_size(pz), fp);
    fwrite(pz->col_clues[0], sizeof(int), pz->n_cols * get_col_clueline_size(pz), fp);

    // Board state
    fwrite(gs->board_state[0], sizeof(enum cell_state), pz->n_rows * pz->n_cols, fp);
    
    fclose(fp);
}

struct game_state *game_state_load(void)
{
    struct puzzle *pz = malloc(sizeof(struct puzzle));
    ALLOC_CHECK_RETURN(pz, NULL);

    FILE *fp = fopen(SAVE_FILE_NAME, "rb");
    if (fp == NULL)
    {
        LOGF(LOG_WARNING, "Failed to open file: %s", SAVE_FILE_NAME);
        free(pz);
        return NULL;
    }

    // Puzzle
    
    fread(pz->title, sizeof(char), MAX_PZ_TITLE_LEN, fp);
    fread(pz->author, sizeof(char), MAX_PZ_AUTHOR_LEN, fp);
    fread(&pz->difficulty, sizeof(int), 1, fp);
    fread(&pz->n_rows, sizeof(int), 1, fp);
    fread(&pz->n_cols, sizeof(int), 1, fp);

    pz->row_clues = (int **)alloc2d(pz->n_rows, get_row_clueline_size(pz), sizeof(int));
    pz->col_clues = (int **)alloc2d(pz->n_cols, get_col_clueline_size(pz), sizeof(int));
    if (pz->row_clues == NULL || pz->col_clues == NULL)
    {
        free(pz);
        fclose(fp);
        return NULL;
    }
    fread(pz->row_clues[0], sizeof(int), pz->n_rows * get_row_clueline_size(pz), fp);
    fread(pz->col_clues[0], sizeof(int), pz->n_cols * get_col_clueline_size(pz), fp);

    struct game_state *gs = game_state_create(pz);
    if (gs == NULL)
    {
        free(pz);
        fclose(fp);
        return NULL;
    }

    gs->board_state = board_state_create(pz);
    fread(gs->board_state[0], sizeof(enum cell_state), pz->n_rows * pz->n_cols, fp);

    fclose(fp);

    return gs;
}

/* Private */

struct undo_queue *undo_queue_create(void)
{
    // head, tail and data entry (especially n_modified) initializes to 0
    struct undo_queue *q = calloc(1, sizeof(struct undo_queue));
    ALLOC_CHECK_EXIT(q);
    return q;
}

void undo_queue_push(struct undo_queue *q, struct undo_entry entry)
{
    assert(q != NULL);

    q->data[q->tail] = entry;
    q->tail = UNDO_NEXT_INDEX(q->tail);
    
    // If queue is full, remove the oldest entry
    if (q->tail == q->head)
    {
        q->data[q->head].n_modified = UNDO_ENTRY_UNMODIFIED;
        q->head = UNDO_NEXT_INDEX(q->head);
    }
}

void set_cell_state_internal(struct game_state *gs, struct cell curr,
                             enum cell_state new_state, int *modified_cnt)
{
    assert(gs != NULL);
    assert(gs->board_state != NULL);
    assert(curr.row >= 0 && curr.row < gs->puzzle->n_rows);
    assert(curr.col >= 0 && curr.col < gs->puzzle->n_cols);

    enum cell_state old_state = get_cell_state(gs, curr);
    if (old_state != new_state)
    {
        if (modified_cnt != NULL)
        {
            (*modified_cnt)++;
            struct undo_entry entry = {curr, old_state, *modified_cnt};
            undo_queue_push(gs->undo_queue, entry);
        }
        gs->board_state[curr.row][curr.col] = new_state;
    }
}

enum cell_state **board_state_create(const struct puzzle *pz)
{
    assert(pz != NULL);
    enum cell_state **board_state;

    board_state = (enum cell_state **)alloc2d(pz->n_rows, pz->n_cols,
                                              sizeof(enum cell_state));
    ALLOC_CHECK_EXIT(board_state);
    for (int i = 0; i < pz->n_rows; i++)
    {
        for (int j = 0; j < pz->n_cols; j++)
        {
            board_state[i][j] = CELL_EMPTY;
        }
    }

    return board_state;
}

