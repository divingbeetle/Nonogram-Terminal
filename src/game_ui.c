#include "game_ui.h"
#include "game_core.h"
#include "utils.h"
#include <curses.h>

/* 1x1 cell */
#define CELL_ROW_TOP "+---"
#define CELL_ROW_MID "|   "
#define CELL_WIDTH 4
#define CELL_HEIGHT 2

/* Function prototypes */ 
int game_ui_set_windows(struct game_ui *ui);
int get_clueline_render_size(const struct puzzle *pz, enum axis axis);

struct pos cell_to_board_pos(struct game_ui *ui, struct cell cell);
struct pos cell_to_win_pos(struct game_ui *ui, struct cell cell);

chtype get_corner_char(struct cell curr, struct cell max);
void draw_1x1_cell_grid(struct game_ui *ui);
void draw_5x5_guide(struct game_ui *ui);
void draw_clues(struct game_ui *ui);

/* Public */ 
struct game_ui *game_ui_create(const struct puzzle *pz)
{
    assert(pz != NULL);

    struct game_ui *ui = malloc(sizeof(struct game_ui));
    ALLOC_CHECK_RETURN(ui, NULL);

    ui->puzzle = pz;
    game_ui_set_windows(ui);
    return ui;
}

void game_ui_destroy(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->win != NULL);
    assert(ui->board != NULL);

    delwin(ui->board);
    delwin(ui->win);

    free(ui);
}

void display_base_board(struct game_ui *ui)
{
    clear();
    refresh();
    box(ui->win, 0, 0);
    draw_1x1_cell_grid(ui);
    draw_5x5_guide(ui);
    draw_clues(ui);
    wrefresh(ui->win);
}

/* Private */

int get_clueline_render_size(const struct puzzle *pz, enum axis axis)
{
    int max_n_valid_clues = 0;
    int **clues = (axis == AXIS_ROW) ? pz->row_clues : pz->col_clues;
    int n_clueline = (axis == AXIS_ROW) ? pz->n_rows : pz->n_cols;
    int clueline_size = (axis == AXIS_ROW) ? get_row_clueline_size(pz) : get_col_clueline_size(pz);

    for (int i = 0; i < n_clueline; i++)
    {
        int n_valid_clues = 0;
        for (int j = 0; j < clueline_size; j++)
        {
            if (clues[i][j] != 0)
            {
                n_valid_clues++;
            }
        }
        max_n_valid_clues = MAX(max_n_valid_clues, n_valid_clues);
    }
    
    int space_per_clue = (axis == AXIS_ROW) ? 3 : 1;
    return max_n_valid_clues * space_per_clue;
}

int game_ui_set_windows(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);

    int win_padding = 1;

    // @TODO: adjust if other windows/elements are needed
    int left_space = 0;
    int top_space = 0;
    int right_space = 0;
    int bottom_space = 0;

    int board_width = ui->puzzle->n_cols * CELL_WIDTH + 1;
    int board_height = ui->puzzle->n_rows * CELL_HEIGHT + 1;
    int row_clues_width = get_clueline_render_size(ui->puzzle, AXIS_ROW);
    int col_clues_height = get_clueline_render_size(ui->puzzle, AXIS_COL);

    struct pos win_size = 
    {
        .y = win_padding * 2 + top_space + col_clues_height + board_height
             + bottom_space,
        .x = win_padding * 2 + left_space + row_clues_width + board_width
             + right_space
    };

    ui->win = newwin(win_size.y, win_size.x, 0, 0);
    ALLOC_CHECK_RETURN(ui->win, -1);
    ui->board = derwin(ui->win, board_height, board_width, 
                       win_padding + top_space + col_clues_height, 
                       win_padding + left_space + row_clues_width);
    ALLOC_CHECK_RETURN(ui->board, -1);

    return 0;
}

struct pos cell_to_board_pos(struct game_ui *ui, struct cell cell)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);

    struct pos board_pos = 
    {
        .y = cell.row * CELL_HEIGHT,
        .x = cell.col * CELL_WIDTH
    };

    return board_pos;
}

struct pos cell_to_win_pos(struct game_ui *ui, struct cell cell)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);
    assert(ui->win != NULL);
    assert(ui->board != NULL);

    struct pos board_pos = cell_to_board_pos(ui, cell);
    struct pos board_start;
    getparyx(ui->board, board_start.y, board_start.x);

    struct pos win_pos = 
    {
        .y = board_start.y + board_pos.y,
        .x = board_start.x + board_pos.x
    };

    return win_pos;
}

void draw_1x1_cell_grid(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);
    assert(ui->board != NULL);

    struct cell curr;
    for (curr.row = 0; curr.row < ui->puzzle->n_rows; curr.row++)
    {
        for (curr.col = 0; curr.col < ui->puzzle->n_cols; curr.col++)
        {
            struct pos board_pos = cell_to_board_pos(ui, curr);
            mvwprintw(ui->board, board_pos.y, board_pos.x, CELL_ROW_TOP);
            mvwprintw(ui->board, board_pos.y + 1, board_pos.x, CELL_ROW_MID);
        }
    }
}

chtype get_corner_char(struct cell curr, struct cell max)
{
    assert(curr.row % 5 == 0 || curr.col % 5 == 0);

    chtype left_corner, right_corner, mid_corner;
    if (curr.row == 0)
    {
        left_corner = ACS_ULCORNER;
        right_corner = ACS_URCORNER;
        mid_corner = ACS_TTEE;
    }
    else if (curr.row == max.row)
    {
        left_corner = ACS_LLCORNER;
        right_corner = ACS_LRCORNER;
        mid_corner = ACS_BTEE;
    }
    else
    {
        left_corner = ACS_LTEE;
        right_corner = ACS_RTEE;
        mid_corner = ACS_PLUS;
    }

    return (curr.col == 0) ? left_corner : 
           (curr.col == max.col) ? right_corner : mid_corner;
}

void draw_5x5_guide(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);
    assert(ui->board != NULL);

    struct cell curr;
    struct cell max = 
    {
        .row = ui->puzzle->n_rows, 
        .col = ui->puzzle->n_cols
    };

    for (curr.row = 0; curr.row <= max.row; curr.row += 5)
    {
        for (curr.col = 0; curr.col <= max.col; curr.col++)
        {
            struct pos board_pos = cell_to_board_pos(ui, curr);
            for (int w = 0; w < CELL_WIDTH; w++)
            {
                mvwaddch(ui->board, board_pos.y, board_pos.x + w, ACS_HLINE);
            }
        }
    }

    for (curr.row = 0; curr.row <= max.row; curr.row++)
    {
        for (curr.col = 0; curr.col <= max.col; curr.col += 5)
        {
            struct pos board_pos = cell_to_board_pos(ui, curr);
            for (int h = 0; h < CELL_HEIGHT; h++)
            {
                mvwaddch(ui->board, board_pos.y + h, board_pos.x, ACS_VLINE);
            }
        }
    }

    for (curr.row = 0; curr.row <= max.row; curr.row += 5)
    {
        for (curr.col = 0; curr.col <= max.col; curr.col += 5)
        {
            struct pos board_pos = cell_to_board_pos(ui, curr);
            mvwaddch(ui->board, board_pos.y, board_pos.x, get_corner_char(curr, max));
        }
    }
}

void draw_clues(struct game_ui *ui)
{
    int row_clues_size = get_row_clueline_size(ui->puzzle);
    int col_clues_size = get_col_clueline_size(ui->puzzle);

    struct cell curr;
    for (curr.row = 0, curr.col = 0; curr.row < ui->puzzle->n_rows; curr.row++)
    {
        struct pos win_pos = cell_to_win_pos(ui, curr);
        win_pos.y += 1; // middle of the cell
        win_pos.x -= 3; // clue width

        for (int i = row_clues_size - 1; i >= 0; i--)
        {
            int clue = ui->puzzle->row_clues[curr.row][i];
            if (clue != 0)
            {
                mvwprintw(ui->win, win_pos.y, win_pos.x, "%3d", clue);
                win_pos.x -= 3;
            }
        }
    }

    for (curr.row = 0, curr.col = 0; curr.col < ui->puzzle->n_cols; curr.col++)
    {
        struct pos win_pos = cell_to_win_pos(ui, curr);
        win_pos.y -= 1; // clue height
        win_pos.x += 1; // middle of the cell
        
        for (int i = col_clues_size - 1; i >= 0; i--)
        {
            int clue = ui->puzzle->col_clues[curr.col][i];
            if (clue != 0)
            {
                mvwprintw(ui->win, win_pos.y, win_pos.x, "%2d", clue);
                win_pos.y -= 1;
            }
        }
    }
}

