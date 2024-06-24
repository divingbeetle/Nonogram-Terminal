#ifndef PUZZLE_H
#define PUZZLE_H

/******************************************************************************
 * NONOGRAM PUZZLE 
 * 
 * Puzzles should be 
 *  - Black/White, square grid nonogram puzzle 
 *  - Size should be multiple of 5
 *
 * Clues should be 
 *  - Right aligned with 0 padding in a array. 
 *****************************************************************************/

#define JSON_FMT_VER "0.2.0"
#define JSON_FMT_VER_LEN 5

#define MAX_PZ_TITLE_LEN     63
#define MAX_PZ_AUTHOR_LEN    63
#define MAX_PZ_DESC_LEN      255
#define MAX_PZ_FILE_NAME_LEN 255

#define MAX_PZ_N_ROWS 50 
#define MAX_PZ_N_COLS 50

#define MAX_PZ_PER_SET 10

struct puzzle_set
{
    char file_name[MAX_PZ_FILE_NAME_LEN + 1];
    char format_ver[JSON_FMT_VER_LEN + 1];
    char title[MAX_PZ_TITLE_LEN + 1];
    char desc[MAX_PZ_DESC_LEN + 1];
    int num_puzzles;
    struct puzzle *puzzles[MAX_PZ_PER_SET];
};

struct puzzle 
{
    char title[MAX_PZ_TITLE_LEN + 1];
    char author[MAX_PZ_AUTHOR_LEN + 1];
    int difficulty;
    int n_rows;
    int n_cols;
    int **row_clues;
    int **col_clues;
};

struct cell 
{
    int row, col;
};

struct puzzle_set *puzzle_set_get_user_choice(void);
struct puzzle *puzzle_get_user_choice(struct puzzle_set *pset);

struct puzzle_set *puzzle_set_load_from_file(const char *file_name);

static inline int get_row_clueline_size(const struct puzzle *pz)
{
    return (pz->n_cols + 1) / 2;
}

static inline int get_col_clueline_size(const struct puzzle *pz)
{
    return (pz->n_rows + 1) / 2;
}

static inline struct cell get_puzzle_size(const struct puzzle *pz)
{
    return (struct cell){pz->n_rows, pz->n_cols};
}

#endif // PUZZLE_H
