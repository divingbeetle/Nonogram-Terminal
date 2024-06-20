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

struct puzzle 
{
    int n_rows;
    int n_cols;
    int **row_clues;
    int **col_clues;
};

/**
 * @TODO: Delete this function when the actual puzzle loading is implemented.
 */
struct puzzle *get_test_puzzle(void);

static inline int get_row_clueline_size(const struct puzzle *pz)
{
    return (pz->n_cols + 1) / 2;
}

static inline int get_col_clueline_size(const struct puzzle *pz)
{
    return (pz->n_rows + 1) / 2;
}


#endif // PUZZLE_H
