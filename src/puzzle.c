#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "loader.h"
#include "puzzle.h"
#include "tui.h"
#include "utils.h"

enum load_mode 
{
    LOAD_METADATA_ONLY,
    LOAD_ALL
};

/* As of ver 0.2.0 */

enum puzzle_json_key 
{
    // Puzzle Set keys
    KEY_PSET_FMT_VER,
    KEY_PSET_TITLE,
    KEY_PSET_DESC,
    KEY_PSET_N_PUZZLES,
    KEY_PSET_PUZZLES,

    // Each Puzzle
    KEY_PZ_TITLE,
    KEY_PZ_AUTHOR,
    KEY_PZ_DIFFICULTY,
    KEY_PZ_ROWS,
    KEY_PZ_COLS,
    KEY_PZ_ROW_CLUES,
    KEY_PZ_COL_CLUES,

    KEY_N_KEYS
};

#define PSET_KEY_START KEY_PSET_FMT_VER
#define PZ_KEY_START   KEY_PZ_TITLE
#define PSET_KEY_END   PZ_KEY_START
#define PZ_KEY_END     KEY_N_KEYS

static const struct json_property puzzle_json_props[KEY_N_KEYS] = 
{
    [KEY_PSET_FMT_VER] = 
    {
        .name = "format_version",
        .type = cJSON_String,
        .min = 1,
        .max = JSON_FMT_VER_LEN
    },
    [KEY_PSET_TITLE] = 
    {
        .name = "title",
        .type = cJSON_String,
        .min = 1,
        .max = MAX_PZ_TITLE_LEN
    },
    [KEY_PSET_DESC] = 
    {
        .name = "description",
        .type = cJSON_String,
        .min = 1,
        .max = MAX_PZ_DESC_LEN
    },
    [KEY_PSET_N_PUZZLES] = 
    {
        .name = "num_puzzles",
        .type = cJSON_Number,
        .min = 1,
        .max = MAX_PZ_PER_SET
    },
    [KEY_PSET_PUZZLES] = 
    {
        .name = "puzzles",
        .type = cJSON_Array,
        .min = 1,
        .max = MAX_PZ_PER_SET
    },
    [KEY_PZ_TITLE] = 
    {
        .name = "title",
        .type = cJSON_String,
        .min = 1,
        .max = MAX_PZ_TITLE_LEN
    },
    [KEY_PZ_AUTHOR] = 
    {
        .name = "author",
        .type = cJSON_String,
        .min = 1,
        .max = MAX_PZ_AUTHOR_LEN
    },
    [KEY_PZ_DIFFICULTY] = 
    {
        .name = "difficulty",
        .type = cJSON_Number,
        .min = 0,
        .max = 10
    },
    [KEY_PZ_ROWS] = 
    {
        .name = "rows",
        .type = cJSON_Number,
        .min = 1,
        .max = MAX_PZ_N_ROWS
    },
    [KEY_PZ_COLS] = 
    {
        .name = "cols",
        .type = cJSON_Number,
        .min = 1,
        .max = MAX_PZ_N_COLS
    },
    [KEY_PZ_ROW_CLUES] = 
    {
        .name = "row_clues",
        .type = cJSON_Array,
        .min = 1,
        .max = MAX_PZ_N_ROWS
    },
    [KEY_PZ_COL_CLUES] = 
    {
        .name = "col_clues",
        .type = cJSON_Array,
        .min = 1,
        .max = MAX_PZ_N_COLS
    }
};

/* Function Prototypes */

bool is_valid_json_puzzle_format(const cJSON *json);
bool is_valid_puzzle_set_metadata(const cJSON *json);
bool is_valid_puzzles(const cJSON *puzzles);

struct puzzle *puzzle_create(const cJSON *json);

int **clues_create(const cJSON *json, const struct puzzle *pz, enum axis axis);

struct puzzle_set *puzzle_set_create(const char *file_name, enum load_mode mode);
void load_puzzle_set_metadata(const cJSON *json, struct puzzle_set *pset);

struct puzzle_set **create_puzzle_set_arr(int *arr_size_out);

bool fread_puzzle(FILE *fp, struct puzzle *pz);

bool is_valid_puzzle(const struct puzzle *pz)
{
    // @TODO: Implement
    return true;
}

/* Public */

void puzzle_set_destroy(struct puzzle_set *pset)
{
    if (pset != NULL)
    {
        for (int i = 0; i < pset->num_puzzles; i++)
        {
            puzzle_destroy(pset->puzzles[i]);
        }
    }
    free(pset);
}

struct puzzle *puzzle_create_from_save(void)
{
    FILE          *fp = NULL;
    struct puzzle *pz = NULL;

    pz = malloc(sizeof(struct puzzle));
    ALLOC_CHECK_RETURN(pz, NULL);

    fp = fopen(SAVE_FILE_NAME, "rb");
    if (fp == NULL)
    {
        LOGF(LOG_ERROR, "Failed to open file: '%s'", SAVE_FILE_NAME);
        free(pz);
        return NULL;
    }

    if (!fread_puzzle(fp, pz))
    {
        LOG(LOG_ERROR, "Failed to read puzzle from Save File");
        free(pz);
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    if (!is_valid_puzzle(pz))
    {
        LOG(LOG_ERROR, "Invalid puzzle loaded from Save File");
        puzzle_destroy(pz);
        return NULL;
    }

    return pz;
}

bool skip_puzzle_from_file(FILE *fp, const struct puzzle *pz)
{
    assert(fp != NULL);
    assert(pz != NULL);

    int offsets[] = 
    {
        sizeof(*(pz->title)) * (MAX_PZ_TITLE_LEN + 1),
        sizeof(*(pz->author)) * (MAX_PZ_AUTHOR_LEN + 1),
        sizeof(pz->difficulty),
        sizeof(pz->n_rows),
        sizeof(pz->n_cols),
        sizeof(int) * pz->n_rows * get_row_clueline_size(pz),
        sizeof(int) * pz->n_cols * get_col_clueline_size(pz)
    };

    int arr_size = sizeof(offsets) / sizeof(*offsets);

    for (int i = 0; i < arr_size; i++)
    {
        if (fseek(fp, offsets[i], SEEK_CUR) != 0)
        {
            LOG(LOG_ERROR, "Failed to skip puzzle from file");
            return false;
        }
    }

    return true;
}

struct puzzle_set *puzzle_set_create_from_user_selection(void)
{
    int n_puzzle_sets;
    struct puzzle_set **puzzle_sets = create_puzzle_set_arr(&n_puzzle_sets);
    if (puzzle_sets == NULL) return NULL;

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
        free_ptr_array((void **) puzzle_sets, n_puzzle_sets);
        return NULL;
    }

    struct puzzle_set *selected_pset;
    selected_pset = puzzle_set_create(puzzle_sets[selected]->file_name, LOAD_ALL);
    free_ptr_array((void **) puzzle_sets, n_puzzle_sets);

    return selected_pset;
}

struct puzzle *select_puzzle_from_set(struct puzzle_set *pset)
{
    if (pset == NULL) return NULL;

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

    return selected_puzzle;
}

void puzzle_destroy(struct puzzle *puzzle)
{
    if (puzzle != NULL)
    {
        free2d((void **) puzzle->row_clues, puzzle->n_rows);
        free2d((void **) puzzle->col_clues, puzzle->n_cols);
    }
    free(puzzle);
}

/* Private */ 

struct puzzle_set **create_puzzle_set_arr(int *arr_size_out)
{
    assert(arr_size_out != NULL);

    char              **file_names = NULL;
    struct puzzle_set **pset_arr   = NULL;
    struct puzzle_set  *pset       = NULL;

    int n_json_files;
    file_names = list_json_files(PUZZLE_DIR, &n_json_files);
    if (file_names == NULL) return NULL;

    pset_arr = malloc(n_json_files * sizeof(struct puzzle_set *));
    if (pset_arr == NULL)
    {
        LOG(LOG_ERROR, "Memory allocation failed");
        free_ptr_array((void **) file_names, n_json_files);
        return NULL;
    }

    // Add valid puzzle sets only
    int n_puzzle_sets = 0;
    for (int i = 0; i < n_json_files; i++)
    {
        pset = puzzle_set_create(file_names[i], LOAD_METADATA_ONLY);
        if (pset != NULL)
        {
            pset_arr[n_puzzle_sets++] = pset;
        }
    }

    *arr_size_out = n_puzzle_sets;
    free_ptr_array((void **) file_names, n_json_files);
    return pset_arr;
}

int **clues_create(const cJSON *json, const struct puzzle *pz, enum axis axis)
{
    assert(json != NULL);
    assert(pz != NULL);

    int axis_size = (axis == AXIS_ROW) ? pz->n_rows : pz->n_cols;
    int clueline_size = (axis == AXIS_ROW) 
                         ? get_row_clueline_size(pz) 
                         : get_col_clueline_size(pz);

    int **clues = (int **) calloc2d(axis_size, clueline_size, sizeof(int));
    ALLOC_CHECK_RETURN(clues, NULL);

    // Right align
    for (int i = 0; i < axis_size; i++)
    {
        cJSON *clues_json = cJSON_GetArrayItem(json, i);
        int arr_size = cJSON_GetArraySize(clues_json);
        int start = clueline_size - arr_size;
        for (int k = 0; k < arr_size; k++)
        {
            clues[i][start + k] = cJSON_GetArrayItem(clues_json, k)->valueint;
        }
    }

    return clues;
}

struct puzzle *puzzle_create(const cJSON *json)
{
    struct puzzle *pz = malloc(sizeof(struct puzzle));
    ALLOC_CHECK_RETURN(pz, NULL);

    cJSON *title      = get_cJSON(json, puzzle_json_props[KEY_PZ_TITLE]);
    cJSON *author     = get_cJSON(json, puzzle_json_props[KEY_PZ_AUTHOR]);
    cJSON *difficulty = get_cJSON(json, puzzle_json_props[KEY_PZ_DIFFICULTY]);
    cJSON *rows       = get_cJSON(json, puzzle_json_props[KEY_PZ_ROWS]);
    cJSON *cols       = get_cJSON(json, puzzle_json_props[KEY_PZ_COLS]);
    cJSON *row_clues  = get_cJSON(json, puzzle_json_props[KEY_PZ_ROW_CLUES]);
    cJSON *col_clues  = get_cJSON(json, puzzle_json_props[KEY_PZ_COL_CLUES]);

    strncpy(pz->title,  title->valuestring,  MAX_PZ_TITLE_LEN + 1);
    strncpy(pz->author, author->valuestring, MAX_PZ_AUTHOR_LEN + 1);

    pz->difficulty = difficulty->valueint;
    pz->n_rows     = rows->valueint;
    pz->n_cols     = cols->valueint;

    pz->row_clues = clues_create(row_clues, pz, AXIS_ROW);
    if (pz->row_clues == NULL)
    {
        free(pz);
        return NULL;
    }

    pz->col_clues = clues_create(col_clues, pz, AXIS_COL);
    if (pz->col_clues == NULL)
    {
        free2d((void **) pz->row_clues, pz->n_rows);
        free(pz);
        return NULL;
    }

    return pz;
}


void load_puzzle_set_metadata(const cJSON *json, struct puzzle_set *pset)
{
    assert(json != NULL);
    assert(pset != NULL);
    assert(pset->file_name != NULL);

    cJSON *fmt_ver   = get_cJSON(json, puzzle_json_props[KEY_PSET_FMT_VER]);
    cJSON *title     = get_cJSON(json, puzzle_json_props[KEY_PSET_TITLE]);
    cJSON *desc      = get_cJSON(json, puzzle_json_props[KEY_PSET_DESC]);
    cJSON *n_puzzles = get_cJSON(json, puzzle_json_props[KEY_PSET_N_PUZZLES]);

    strncpy(pset->format_ver, fmt_ver->valuestring, JSON_FMT_VER_LEN + 1);
    strncpy(pset->title,      title->valuestring,   MAX_PZ_TITLE_LEN + 1);
    strncpy(pset->desc,       desc->valuestring,    MAX_PZ_DESC_LEN + 1);
    pset->num_puzzles = n_puzzles->valueint;
}

struct puzzle_set *puzzle_set_create(const char *file_name, enum load_mode mode)
{
    struct puzzle_set *pset = NULL;
    cJSON             *json = NULL;

    int n_puzzles_created = 0;

    json = cJSON_parse_file(file_name);
    if (json == NULL) return NULL;

    if (!is_valid_json_puzzle_format(json))
    {
        LOGF(LOG_WARNING, "Invalid JSON format in file: %s", file_name);
        goto cleanup;
    }

    pset = malloc(sizeof(struct puzzle_set));
    ALLOC_CHECK_RETURN(pset, NULL);

    strncpy(pset->file_name, file_name, MAX_PZ_FILE_NAME_LEN);
    load_puzzle_set_metadata(json, pset);

    n_puzzles_created = 0;
    if (mode != LOAD_METADATA_ONLY)
    {
        struct puzzle *pz;
        cJSON *puzzles_json;
        cJSON *pz_json;
        puzzles_json = get_cJSON(json, puzzle_json_props[KEY_PSET_PUZZLES]);
        for (int i = 0; i < pset->num_puzzles; i++)
        {
            pz_json = cJSON_GetArrayItem(puzzles_json, i);
            pz = puzzle_create(pz_json);
            if (pz == NULL)
            {
                goto cleanup;
            }
            pset->puzzles[i] = pz;
            n_puzzles_created++;
        }
    }

    cJSON_Delete(json);
    return pset;

cleanup:
    cJSON_Delete(json);
    if (pset != NULL)
    {
        for (int i = 0; i < n_puzzles_created; i++)
        {
            puzzle_destroy(pset->puzzles[i]);
        }
        free(pset);
    }
    return NULL;
}

bool is_valid_json_puzzle_format(const cJSON *json)
{
    if (is_valid_puzzle_set_metadata(json))
    {
        cJSON *puzzles = get_cJSON(json, puzzle_json_props[KEY_PSET_PUZZLES]);
        if (is_valid_puzzles(puzzles))
        {
            return true;
        }
    }
    return false; 
}

bool is_valid_puzzle_set_metadata(const cJSON *json)
{
    assert(json != NULL);

    for (int i = PSET_KEY_START; i < PSET_KEY_END; i++)
    {
        if (!is_valid_json_property(json, puzzle_json_props[i]))
        {
            return false;
        }
    }

    cJSON *fmt_ver = get_cJSON(json, puzzle_json_props[KEY_PSET_FMT_VER]);
    if (strcmp(fmt_ver->valuestring, JSON_FMT_VER) != 0)
    {
        LOGF(LOG_WARNING, 
             "Invalid JSON format version: Got %s, Expected %s",
              fmt_ver->valuestring);
        return false;
    }
    return true;
}

bool is_valid_puzzles(const cJSON *puzzles)
{
    assert(puzzles != NULL);

    const cJSON *pz;
    cJSON_ArrayForEach(pz, puzzles)
    {
        if (!cJSON_IsObject(pz))
        {
            LOG(LOG_WARNING, "Invalid JSON puzzle object");
        }

        for (int i = PZ_KEY_START; i < PZ_KEY_END; i++)
        {
            if (!is_valid_json_property(pz, puzzle_json_props[i]))
            {
                return false;
            }
        }
    }

    return true;
}

bool fread_puzzle(FILE *fp, struct puzzle *pz)
{
    if (!fread(pz->title, sizeof(char), MAX_PZ_TITLE_LEN + 1, fp)
        || !fread(pz->author, sizeof(char), MAX_PZ_AUTHOR_LEN + 1, fp)
        || !fread(&pz->difficulty, sizeof(int), 1, fp)
        || !fread(&pz->n_rows, sizeof(int), 1, fp)
        || !fread(&pz->n_cols, sizeof(int), 1, fp))
    {
        return false;
    }
    pz->title[MAX_PZ_TITLE_LEN]   = '\0';
    pz->author[MAX_PZ_AUTHOR_LEN] = '\0';

    if (pz->n_rows > MAX_PZ_N_ROWS || pz->n_cols > MAX_PZ_N_COLS
        || pz->n_cols < 0 || pz->n_rows < 0)
    {
        return false;
    }

    pz->row_clues = (int **)alloc2d(pz->n_rows, 
                                    get_row_clueline_size(pz), sizeof(int));

    ALLOC_CHECK_RETURN(pz->row_clues, false);
    pz->col_clues = (int **)alloc2d(pz->n_cols, 
                                    get_col_clueline_size(pz), sizeof(int));
    if (pz->col_clues == NULL)
    {
        free2d((void **) pz->row_clues, pz->n_rows);
        return false;
    }

    int row_clues_size = pz->n_rows * get_row_clueline_size(pz);
    int col_clues_size = pz->n_cols * get_col_clueline_size(pz);
    if (!fread(pz->row_clues[0], sizeof(int), row_clues_size, fp)
        || !fread(pz->col_clues[0], sizeof(int), col_clues_size, fp))
    {
        return false;
    }

    return true;
}

