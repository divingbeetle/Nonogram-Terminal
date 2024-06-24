#include "puzzle.h"
#include "config.h"
#include "loader.h"
#include "utils.h"
#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum load_mode 
{
    LOAD_LAZY,
    LOAD_EAGER
};

/* As of ver 0.2.0 */

enum puzzle_set_attr 
{
    PSET_ATTR_FMT_VER,
    PSET_ATTR_TITLE,
    PSET_ATTR_DESC,
    PSET_ATTR_N_PUZZLES,
    PSET_ATTR_PUZZLES,
    PSET_ATTR_N
};

enum puzzle_attr 
{
    PZ_ATTR_TITLE,
    PZ_ATTR_AUTHOR,
    PZ_ATTR_DIFFICULTY,
    PZ_ATTR_ROWS,
    PZ_ATTR_COLS,
    PZ_ATTR_ROW_CLUES,
    PZ_ATTR_COL_CLUES,
    PZ_ATTR_N
};

static struct json_attr puzzle_set_attrs[PSET_ATTR_N] = 
{
    [PSET_ATTR_FMT_VER] = {"format_version", cJSON_String, 1, JSON_FMT_VER_LEN},
    [PSET_ATTR_TITLE]   = {"title",          cJSON_String, 1, MAX_PZ_TITLE_LEN},
    [PSET_ATTR_DESC]    = {"description",    cJSON_String, 1, MAX_PZ_DESC_LEN },
    [PSET_ATTR_N_PUZZLES] = {"num_puzzles",  cJSON_Number, 1, MAX_PZ_PER_SET  },
    [PSET_ATTR_PUZZLES]   = {"puzzles",      cJSON_Array,  1, MAX_PZ_PER_SET  }
};

static struct json_attr puzzle_attrs[PZ_ATTR_N] = 
{
    [PZ_ATTR_TITLE]      = {"title",      cJSON_String, 1, MAX_PZ_TITLE_LEN },
    [PZ_ATTR_AUTHOR]     = {"author",     cJSON_String, 1, MAX_PZ_AUTHOR_LEN},
    [PZ_ATTR_DIFFICULTY] = {"difficulty", cJSON_Number, 0, 10               },
    [PZ_ATTR_ROWS]       = {"rows",       cJSON_Number, 1, MAX_PZ_N_ROWS    },
    [PZ_ATTR_COLS]       = {"cols",       cJSON_Number, 1, MAX_PZ_N_COLS    },
    [PZ_ATTR_ROW_CLUES]  = {"row_clues",  cJSON_Array,  1, MAX_PZ_N_ROWS    },
    [PZ_ATTR_COL_CLUES]  = {"col_clues",  cJSON_Array,  1, MAX_PZ_N_COLS    }
};

/* Function Prototypes */

bool validate_puzzle_set_metadata(const cJSON *json, const char *file_name);

bool validate_puzzles(const cJSON *puzzles, const char *file_name);

void puzzle_destroy(struct puzzle *puzzle);

struct puzzle *puzzle_load(const cJSON *json);
struct puzzle_set *puzzle_set_load(const cJSON *json, const char *file_name, enum load_mode mode);

struct puzzle_set *parse_puzzle_set(const char *file_name, enum load_mode mode);
struct puzzle_set **create_puzzle_set_arr(int *n_out);

/* Public */
struct puzzle_set *puzzle_set_get_user_choice(void)
{
    int n_puzzle_sets;
    struct puzzle_set **puzzle_sets = create_puzzle_set_arr(&n_puzzle_sets);
    if (puzzle_sets == NULL)
    {
        return NULL;
    }

    char *choices[n_puzzle_sets];
    for (int i = 0; i < n_puzzle_sets; i++)
    {
        choices[i] = puzzle_sets[i]->title;
    }

    char *descriptions[n_puzzle_sets];
    for (int i = 0; i < n_puzzle_sets; i++)
    {
        descriptions[i] = puzzle_sets[i]->desc;
    }

    struct menu_param param = 
    {
        .title = "Choose a puzzle set",
        .choices = choices,
        .n_choices = n_puzzle_sets,
        .descriptions = descriptions,
        .start = {0, 0},
        .size = {0, 0}
    };

    struct menu_config config = menu_config_default;
    struct menu_set *mset = menu_set_create(&param);
    menu_set_configure(mset, config); 

    int selected = menu_set_get_user_choice(mset);
    menu_set_destroy(mset);
    if (selected == MENU_NOT_SELECTED)
    {
        return NULL;
    }

    struct puzzle_set *selected_pset;
    selected_pset = parse_puzzle_set(puzzle_sets[selected]->file_name, LOAD_EAGER);
    for (int i = 0; i < n_puzzle_sets; i++)
    {
        free(puzzle_sets[i]);
    }
    free(puzzle_sets);


    return selected_pset;
}

struct puzzle *puzzle_get_user_choice(struct puzzle_set *pset)
{
    if (pset == NULL)
    {
        return NULL;
    }

    int n_puzzle = pset->num_puzzles;

    char *choices[n_puzzle];
    for (int i = 0; i < n_puzzle; i++)
    {
        choices[i] = pset->puzzles[i]->title;
    }

    struct menu_param param = 
    {
        .title = "Choose a puzzle set",
        .choices = choices,
        .n_choices = n_puzzle,
        .descriptions = NULL,
        .start = {0, 0},
        .size = {0, 0}
    };

    struct menu_config config = menu_config_default;
    struct menu_set *mset = menu_set_create(&param);
    menu_set_configure(mset, config); 

    int selected = menu_set_get_user_choice(mset);
    menu_set_destroy(mset);
    if (selected == MENU_NOT_SELECTED)
    {
        return NULL;
    }

    struct puzzle *selected_puzzle = pset->puzzles[selected];

    for (int i = 0; i < n_puzzle; i++)
    {
        if (i != selected)
        {
            free(pset->puzzles[i]);
        }
    }

    return selected_puzzle;
}

/* Private */ 

struct puzzle_set **create_puzzle_set_arr(int *n_out)
{
    assert(n_out != NULL);
    char **files_list;
    int n_files;

    files_list = get_json_files_list(PUZZLE_DIR, &n_files);
    if (files_list == NULL)
    {
        return NULL;
    }

    struct puzzle_set **puzzle_sets = malloc(n_files * sizeof(struct puzzle_set *));
    if (puzzle_sets == NULL)
    {
        LOG(LOG_ERROR, "Memory allocation failed");
        free_ptr_array((void **) files_list, n_files);
        return NULL;
    }

    *n_out = 0;
    for (int i = 0; i < n_files; i++)
    {
        struct puzzle_set *puzzle_set = parse_puzzle_set(files_list[i], LOAD_LAZY);
        if (puzzle_set == NULL)
        {
            LOG(LOG_ERROR, "Failed to parse puzzle set");
            continue;
        }

        puzzle_sets[(*n_out)++] = puzzle_set;
    }

    free_ptr_array((void **) files_list, n_files);
    return puzzle_sets;
}

bool validate_puzzle_set_metadata(const cJSON *json, const char *file_name)
{
    assert(file_name != NULL);
    assert(json != NULL);

    for (int i = 0; i < PSET_ATTR_N; i++)
    {
        if (!validate_attr(json, puzzle_set_attrs[i], file_name))
            return false;
    }

    cJSON *fmt_ver = get_cJSON(json, puzzle_set_attrs[PSET_ATTR_FMT_VER]);
    if (strcmp(fmt_ver->valuestring, JSON_FMT_VER) != 0)
    {
        LOGF(LOG_WARNING, 
             "%s - Invalid JSON format version: Got %s, Expected %s",
             file_name, fmt_ver->valuestring);
        return false;
    }
    return true;
}

bool validate_puzzles(const cJSON *puzzles, const char *file_name)
{
    assert(file_name != NULL);
    assert(puzzles != NULL);

    const cJSON *pz;
    cJSON_ArrayForEach(pz, puzzles)
    {
        if (!cJSON_IsObject(pz))
        {
            LOGF(LOG_WARNING, 
                 "%s - Invalid JSON attribute: %s", 
                 file_name, puzzle_set_attrs[PSET_ATTR_PUZZLES].name);
            return false;
        }

        for (int i = 0; i < PZ_ATTR_N; i++)
        {
            if (!validate_attr(pz, puzzle_attrs[i], file_name))
                return false;
        }
    }

    return true;
}

void puzzle_destroy(struct puzzle *puzzle)
{
    if (puzzle != NULL)
    {
        free2d((void **) puzzle->row_clues, puzzle->n_rows);
        free2d((void **) puzzle->col_clues, puzzle->n_cols);
        free(puzzle);
    }
}

struct puzzle *puzzle_load(const cJSON *json)
{
    struct puzzle *pz = malloc(sizeof(struct puzzle));
    ALLOC_CHECK_RETURN(pz, NULL);

    cJSON *title      = get_cJSON(json, puzzle_attrs[PZ_ATTR_TITLE]);
    cJSON *author     = get_cJSON(json, puzzle_attrs[PZ_ATTR_AUTHOR]);
    cJSON *difficulty = get_cJSON(json, puzzle_attrs[PZ_ATTR_DIFFICULTY]);
    cJSON *rows       = get_cJSON(json, puzzle_attrs[PZ_ATTR_ROWS]);
    cJSON *cols       = get_cJSON(json, puzzle_attrs[PZ_ATTR_COLS]);
    cJSON *row_clues  = get_cJSON(json, puzzle_attrs[PZ_ATTR_ROW_CLUES]);
    cJSON *col_clues  = get_cJSON(json, puzzle_attrs[PZ_ATTR_COL_CLUES]);

    strncpy(pz->title, title->valuestring, MAX_PZ_TITLE_LEN);
    strncpy(pz->author, author->valuestring, MAX_PZ_AUTHOR_LEN);

    pz->difficulty = difficulty->valueint;
    pz->n_rows     = rows->valueint;
    pz->n_cols     = cols->valueint;

    int row_clueline_size = get_row_clueline_size(pz);
    int col_clueline_size = get_col_clueline_size(pz);

    pz->row_clues = (int **) alloc2d(pz->n_rows, row_clueline_size, 
                                     sizeof(int));
    pz->col_clues = (int **) alloc2d(pz->n_cols, col_clueline_size, 
                                     sizeof(int));

    ALLOC_CHECK_RETURN(pz->row_clues, NULL);
    ALLOC_CHECK_RETURN(pz->col_clues, NULL);

    // Right align clues
    for (int i = 0; i < pz->n_rows; i++)
    {
        cJSON *clueline = cJSON_GetArrayItem(row_clues, i);
        int arr_size = cJSON_GetArraySize(clueline);
        int start = row_clueline_size - arr_size;
        for (int k = 0; k < arr_size; k++)
        {
            pz->row_clues[i][start + k] = cJSON_GetArrayItem(clueline, k)->valueint;
        }
    }

    for (int i = 0; i < pz->n_cols; i++)
    {
        cJSON *clueline = cJSON_GetArrayItem(col_clues, i);
        int arr_size = cJSON_GetArraySize(clueline);
        int start = col_clueline_size - arr_size;
        for (int k = 0; k < arr_size; k++)
        {
            pz->col_clues[i][start + k] = cJSON_GetArrayItem(clueline, k)->valueint;
        }
    }

    return pz;
}

struct puzzle_set *puzzle_set_load(const cJSON *json, const char *file_name, enum load_mode mode)
{
    assert(json != NULL);

    struct puzzle_set *pset = malloc(sizeof(struct puzzle_set));
    ALLOC_CHECK_RETURN(pset, NULL);

    cJSON *fmt_ver   = get_cJSON(json, puzzle_set_attrs[PSET_ATTR_FMT_VER]);
    cJSON *title     = get_cJSON(json, puzzle_set_attrs[PSET_ATTR_TITLE]);
    cJSON *desc      = get_cJSON(json, puzzle_set_attrs[PSET_ATTR_DESC]);
    cJSON *n_puzzles = get_cJSON(json, puzzle_set_attrs[PSET_ATTR_N_PUZZLES]);

    strncpy(pset->file_name, file_name, MAX_PZ_FILE_NAME_LEN);
    strncpy(pset->format_ver, fmt_ver->valuestring, JSON_FMT_VER_LEN);
    strncpy(pset->title, title->valuestring, MAX_PZ_TITLE_LEN);
    strncpy(pset->desc, desc->valuestring, MAX_PZ_DESC_LEN);
    pset->num_puzzles = n_puzzles->valueint;

    if (mode != LOAD_LAZY)
    {
        cJSON *puzzles = get_cJSON(json, puzzle_set_attrs[PSET_ATTR_PUZZLES]);
        for (int i = 0; i < pset->num_puzzles; i++)
        {
            cJSON *pz = cJSON_GetArrayItem(puzzles, i);
            pset->puzzles[i] = puzzle_load(pz);
            if (pset->puzzles[i] == NULL)
            {
                for (int k = 0; k < i; k++)
                {
                    puzzle_destroy(pset->puzzles[k]);
                }
                free(pset);
                return NULL;
            }
        }
    }

    return pset;
}

struct puzzle_set *parse_puzzle_set(const char *file_name, enum load_mode mode)
{
    char *file_txt = load_file(file_name);
    if (file_txt == NULL)
    {
        return NULL;
    }

    cJSON *json = cJSON_Parse(file_txt);
    if (json == NULL)
    {
        // @TODO: detailed error message
        free(file_txt);
        return NULL;
    }

    if (!validate_puzzle_set_metadata(json, file_name))
    {
        cJSON_Delete(json);
        free(file_txt);
        return NULL;
    }

    if (mode != LOAD_LAZY)
    {
        cJSON *puzzles = get_cJSON(json, puzzle_set_attrs[PSET_ATTR_PUZZLES]);
        if (!validate_puzzles(puzzles, file_name))
        {
            cJSON_Delete(json);
            free(file_txt);
            return NULL;
        }
    }

    struct puzzle_set *puzzle_set = puzzle_set_load(json, file_name, mode);
    if (puzzle_set == NULL)
    {
        cJSON_Delete(json);
        free(file_txt);
        return NULL;
    }

    return puzzle_set;
}

