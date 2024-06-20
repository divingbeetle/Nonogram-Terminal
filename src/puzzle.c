#include "puzzle.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

struct puzzle *get_test_puzzle(void)
{
    struct puzzle *p = malloc(sizeof(struct puzzle));
    ALLOC_CHECK_EXIT(p);
    
    p->n_rows = 5;
    p->n_cols = 5;
    p->row_clues = (int **) alloc2d(p->n_rows, get_row_clueline_size(p), sizeof(int));
    ALLOC_CHECK_EXIT(p->row_clues);
    p->col_clues = (int **) alloc2d(p->n_cols, get_col_clueline_size(p), sizeof(int));
    ALLOC_CHECK_EXIT(p->col_clues);

    p->row_clues[0][1] = 1;
    p->row_clues[0][2] = 1;
    p->row_clues[1][2] = 5;
    p->row_clues[2][2] = 5;
    p->row_clues[3][2] = 3;
    p->row_clues[4][2] = 1;

    p->col_clues[0][2] = 2;
    p->col_clues[1][2] = 4;
    p->col_clues[2][2] = 4;
    p->col_clues[3][2] = 4;
    p->col_clues[4][2] = 2;

    return p;
}
