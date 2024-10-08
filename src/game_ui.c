#include "game_ui.h"
#include "game_control.h"
#include "tui.h"
#include "utils.h"
#include <curses.h>

/* 1x1 cell grid */
#define CELL_ROW_TOP "+---"
#define CELL_ROW_MID "|   "
#define CELL_WIDTH 4
#define CELL_HEIGHT 2

#define UI_WIN_PADDING 1

#define NTERM_COLOR_FILLED       COLOR_P_BLUE
#define NTERM_COLOR_CLUE_CORRECT COLOR_P_GREEN_HIGHLIGHTED
#define NTERM_COLOR_DEFAULT      COLOR_P_DEFAULT

/* Function prototypes */ 
int get_clueline_render_size(const struct puzzle *pz, enum axis axis);

/**
 * @return position of the cell's top-left corner relative to the board window
 */
struct pos cell_to_board_pos(struct game_ui *ui, struct cell cell);

/**
 * @return position of the cell's top-left corner relative to the main window
 */
struct pos cell_to_win_pos(struct game_ui *ui, struct cell cell);

void draw_1x1_cell_grid(struct game_ui *ui);
void draw_5x5_guide_grid(struct game_ui *ui);
void draw_clues(struct game_ui *ui);
chtype get_corner_char(struct cell curr, struct cell max);

void draw_board_state(struct game_ui *ui, const struct game_state *state);
void colorize_correct_rows(struct game_ui *ui, const struct game_state *state);
void colorize_correct_cols(struct game_ui *ui, const struct game_state *state);
void draw_cell(struct game_ui *ui, struct cell cell, enum cell_state state);
void highlight_cell(struct game_ui *ui, struct cell cell, short color_p);

/* Public */ 
struct game_ui *game_ui_create(const struct puzzle *pz)
{
    assert(pz != NULL);

    struct game_ui *ui = malloc(sizeof(struct game_ui));
    ALLOC_CHECK_EXIT(ui);

    ui->puzzle = pz;
    ui->win = NULL;
    ui->board = NULL;
    ui->cmd_menu = NULL;
    /*game_ui_set_windows(ui);*/
    return ui;
}

void game_ui_destroy(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->win != NULL);
    assert(ui->board != NULL);

    // This function touchs menu->param which was stack allocated
    // Which was temporary solution to creating menu_set 
    // @TODO: Fix
    /*menu_set_destroy(ui->cmd_menu);*/
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
    draw_5x5_guide_grid(ui);
    draw_clues(ui);
    wrefresh(ui->win);
}

void highlight_area(struct game_ui *ui, struct cell start, struct cell end, attr_t attr)
{
    struct cell curr;
    for (curr.row = start.row; curr.row <= end.row; curr.row++)
    {
        for (curr.col = start.col; curr.col <= end.col; curr.col++)
        {
            highlight_cell(ui, curr, attr);
        }
    }
}

void display_game_state(struct game_ui *ui, const struct game_state *state)
{
    draw_board_state(ui, state);
    colorize_correct_rows(ui, state);
    colorize_correct_cols(ui, state);
    wrefresh(ui->board);
}

/* Private */

int get_clueline_render_size(const struct puzzle *pz, enum axis axis)
{
    int max_n_valid_clues = 0;
    int **clues           = (axis == AXIS_ROW) ? pz->row_clues : pz->col_clues;
    int n_clueline        = (axis == AXIS_ROW) ? pz->n_rows : pz->n_cols;
    int clueline_size     = (axis == AXIS_ROW) ? get_row_clueline_size(pz)
                                               : get_col_clueline_size(pz);

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
    
    // Row : 3 one for space, two for number (max 2 digits)  
    // Col : 1 one for number since it's vertical
    int space_per_clue = (axis == AXIS_ROW) ? 3 : 1;
    return max_n_valid_clues * space_per_clue;
}

int game_ui_set_windows(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);

    int win_padding = UI_WIN_PADDING;

    // @TODO: adjust if other windows/elements are needed
    int left_space   = 0;
    int top_space    = 0;
    int right_space  = 0;
    int bottom_space = 0;

    int board_width      = ui->puzzle->n_cols * CELL_WIDTH + 1;
    int board_height     = ui->puzzle->n_rows * CELL_HEIGHT + 1;
    int row_clues_width  = get_clueline_render_size(ui->puzzle, AXIS_ROW);
    int col_clues_height = get_clueline_render_size(ui->puzzle, AXIS_COL);

    struct menu_param params =
    {
        .title = "COMMAND MODE",
        .size = {.x = 0, .y = 0},
        .start = {.x = win_padding + left_space + row_clues_width + board_width,
                .y = win_padding + top_space + col_clues_height + 1},
        .n_choices = CMD_N,
        .choices = command_mode_choices,
        .descriptions = command_mode_desc,
    };

    struct menu_config config = menu_config_default;

    ui->cmd_menu = menu_set_create(&params);
    ALLOC_CHECK_EXIT(ui->cmd_menu);
    menu_set_configure(ui->cmd_menu, config);

    right_space = getmaxx(ui->cmd_menu->win);

    struct pos win_size = 
    {
        .y = win_padding * 2 + top_space + col_clues_height + board_height
             + bottom_space,
        .x = win_padding * 2 + left_space + row_clues_width + board_width
             + right_space
    };

    ui->win = newwin(win_size.y, win_size.x, 0, 0);
    ALLOC_CHECK_EXIT(ui->win);

    ui->board = derwin(ui->win, board_height, board_width, 
                       win_padding + top_space + col_clues_height, 
                       win_padding + left_space + row_clues_width);
    ALLOC_CHECK_EXIT(ui->board);

    return 0;
}

struct pos cell_to_board_pos(struct game_ui *ui, struct cell cell)
{
    struct pos cell_pos = 
    {
        .y = cell.row * CELL_HEIGHT,
        .x = cell.col * CELL_WIDTH
    };

    return cell_pos;
}

struct pos cell_to_win_pos(struct game_ui *ui, struct cell cell)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);
    assert(ui->win != NULL);
    assert(ui->board != NULL);

    struct pos cell_board_pos = cell_to_board_pos(ui, cell);
    struct pos board_start;
    getparyx(ui->board, board_start.y, board_start.x);

    struct pos cell_win_pos = 
    {
        .y = board_start.y + cell_board_pos.y,
        .x = board_start.x + cell_board_pos.x
    };

    return cell_win_pos;
}

void draw_1x1_cell_grid(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);
    assert(ui->board != NULL);

    struct cell curr;
    struct cell max = get_puzzle_size(ui->puzzle);
    // Does not draw edges properly, but 5x5 grid will override anyway
    for (curr.row = 0; curr.row < max.row; curr.row++)
    {
        for (curr.col = 0; curr.col < max.col; curr.col++)
        {
            struct pos cell_pos = cell_to_board_pos(ui, curr);
            mvwprintw(ui->board, cell_pos.y, cell_pos.x, CELL_ROW_TOP);
            mvwprintw(ui->board, cell_pos.y + 1, cell_pos.x, CELL_ROW_MID);
        }
    }
}

chtype get_corner_char(struct cell curr, struct cell max)
{
    assert(curr.row % 5 == 0 || curr.col % 5 == 0);

    chtype left_corner, right_corner, mid_corner;
    if (curr.row == 0)
    {
        left_corner  = ACS_ULCORNER;
        right_corner = ACS_URCORNER;
        mid_corner   = ACS_TTEE;
    }
    else if (curr.row == max.row)
    {
        left_corner  = ACS_LLCORNER;
        right_corner = ACS_LRCORNER;
        mid_corner   = ACS_BTEE;
    }
    else
    {
        left_corner  = ACS_LTEE;
        right_corner = ACS_RTEE;
        mid_corner   = ACS_PLUS;
    }

    return (curr.col == 0) ? left_corner 
                           : (curr.col == max.col) ? right_corner 
                                                   : mid_corner;
}

void draw_5x5_guide_grid(struct game_ui *ui)
{
    assert(ui != NULL);
    assert(ui->puzzle != NULL);
    assert(ui->board != NULL);

    struct cell curr;
    struct pos cell_pos;
    struct cell max = get_puzzle_size(ui->puzzle);

    // Horizontal Guideline
    for (curr.row = 0; curr.row <= max.row; curr.row += 5)
    {
        for (curr.col = 0; curr.col <= max.col; curr.col++)
        {
            cell_pos = cell_to_board_pos(ui, curr);
            for (int w = 0; w < CELL_WIDTH; w++)
            {
                mvwaddch(ui->board, cell_pos.y, cell_pos.x + w, ACS_HLINE);
            }
        }
    }

    // Vertical Guideline
    for (curr.row = 0; curr.row <= max.row; curr.row++)
    {
        for (curr.col = 0; curr.col <= max.col; curr.col += 5)
        {
            cell_pos = cell_to_board_pos(ui, curr);
            for (int h = 0; h < CELL_HEIGHT; h++)
            {
                mvwaddch(ui->board, cell_pos.y + h, cell_pos.x, ACS_VLINE);
            }
        }
    }

    // Corners
    for (curr.row = 0; curr.row <= max.row; curr.row += 5)
    {
        for (curr.col = 0; curr.col <= max.col; curr.col += 5)
        {
            chtype corner_char = get_corner_char(curr, max);
            cell_pos = cell_to_board_pos(ui, curr);
            mvwaddch(ui->board, cell_pos.y, cell_pos.x, corner_char);
        }
    }
}

void draw_clues(struct game_ui *ui)
{
    int row_clues_size = get_row_clueline_size(ui->puzzle);
    int col_clues_size = get_col_clueline_size(ui->puzzle);

    struct cell curr = {0, 0};
    for (; curr.row < ui->puzzle->n_rows; curr.row++)
    {
        struct pos win_pos = cell_to_win_pos(ui, curr);
        for (int i = row_clues_size - 1; i >= 0; i--)
        {
            int clue = ui->puzzle->row_clues[curr.row][i];
            if (clue != 0)
            {
                // y + 1 for middle of the cell height 
                // x - 3 for space + digits width
                mvwprintw(ui->win, win_pos.y + 1, win_pos.x - 3, "%3d", clue);
                win_pos.x -= 3;
            }
        }
    }

    curr = (struct cell){0, 0};
    for (; curr.col < ui->puzzle->n_cols; curr.col++)
    {
        struct pos win_pos = cell_to_win_pos(ui, curr);
        for (int i = col_clues_size - 1; i >= 0; i--)
        {
            int clue = ui->puzzle->col_clues[curr.col][i];
            if (clue != 0)
            {
                // y - 1 for space above the board 
                // x + 1 to make single digit clue to be in the middle
                mvwprintw(ui->win, win_pos.y - 1, win_pos.x + 1, "%2d", clue);
                win_pos.y -= 1;
            }
        }
    }
}

void draw_cell(struct game_ui *ui, struct cell cell, enum cell_state state)
{
    struct pos cell_pos = cell_to_board_pos(ui, cell);

    // Altchar
    if (state == CELL_FILLED)
    {
        wattron(ui->board, COLOR_PAIR(NTERM_COLOR_FILLED));
        for (int dx = 1; dx < CELL_WIDTH; dx++)
        {
            mvwaddch(ui->board, cell_pos.y + 1, cell_pos.x + dx, ACS_CKBOARD);
        }
        wattroff(ui->board, COLOR_PAIR(NTERM_COLOR_FILLED));
        return;
    }

    // Normal char
    char *cell_state_str;
    switch (state)
    {
        case CELL_EMPTY:
            cell_state_str = "   ";
            break;
        case CELL_XMARKED:
            cell_state_str = "XXX";
            break;
        case CELL_TEMP_FILLED:
            cell_state_str = " . ";
            break;
        case CELL_TEMP_XMARKED:
            cell_state_str = " x ";
            break;
        default:
            LOGF(LOG_WARNING, "Unhandled cell_state: %d", state);
            cell_state_str = "   ";
            break;
    }
    mvwprintw(ui->board, cell_pos.y + 1, cell_pos.x + 1, "%s", cell_state_str);
}

void highlight_cell(struct game_ui *ui, struct cell cell, short color_p)
{
    assert(ui != NULL);
    assert(ui->board != NULL);

    struct pos curr = cell_to_board_pos(ui, cell);
    for (int dy = 0; dy <= CELL_HEIGHT; dy++)
    {
        for (int dx = 0; dx <= CELL_WIDTH; dx++)
        {
            attr_t curr_attr = A_NORMAL;
            // Altcharset/normal char should be handled differently
            if (mvwinch(ui->board, curr.y + dy, curr.x + dx) & A_ALTCHARSET)
            {
                curr_attr |= A_ALTCHARSET;
            }

            mvwchgat(ui->board, 
                     curr.y + dy, curr.x + dx, 1, curr_attr, color_p, NULL);
        }
    }
}

void draw_board_state(struct game_ui *ui, const struct game_state *state)
{
    struct cell curr;
    for (curr.row = 0; curr.row < ui->puzzle->n_rows; curr.row++)
    {
        for (curr.col = 0; curr.col < ui->puzzle->n_cols; curr.col++)
        {
            enum cell_state cell_state = get_cell_state(state, curr);
            draw_cell(ui, curr, cell_state);
        }
    }
}

void colorize_correct_rows(struct game_ui *ui, const struct game_state *state)
{
    int left_padding = UI_WIN_PADDING;
    for (int i = 0; i < ui->puzzle->n_rows; i++)
    {
        /*attr_t attr = validate_axis(state, AXIS_ROW, i) ? A_REVERSE : A_NORMAL;*/
        short color_p = validate_axis(state, AXIS_ROW, i) ? NTERM_COLOR_CLUE_CORRECT : NTERM_COLOR_DEFAULT;

        struct cell curr   = {i, 0};
        struct pos win_pos = cell_to_win_pos(ui, curr);
        mvwchgat(ui->win, 
                 win_pos.y + 1, left_padding, 
                 win_pos.x - left_padding, A_NORMAL, color_p, NULL);
    }
}
void colorize_correct_cols(struct game_ui *ui, const struct game_state *state)
{
    int top_padding = UI_WIN_PADDING;
    for (int j = 0; j < ui->puzzle->n_cols; j++)
    {
        short color_p = validate_axis(state, AXIS_COL, j) ? NTERM_COLOR_CLUE_CORRECT : NTERM_COLOR_DEFAULT;

        struct cell curr   = {0, j};
        struct pos win_pos = cell_to_win_pos(ui, curr);
        for (int y = top_padding; y < win_pos.y; y++)
        {
            mvwchgat(ui->win, y, win_pos.x + 1, 3, A_NORMAL, color_p, NULL);
        }
    }
}
