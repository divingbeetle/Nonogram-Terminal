#ifndef LOADER_H
#define LOADER_H

#include <cJSON/cJSON.h>
#include <stdbool.h>

struct json_attr
{
    const char *name;
    int type;
    int min, max;
};

/**
 * Load a text file 
 * @retval NULL if error
 */
char *load_file(const char *file_name);


char **get_json_files_list(const char *dir_name, int *n_files_out);

bool validate_attr(const cJSON *json, struct json_attr attr, const char *file_name);

/** 
 * Wrapper for cJSON_GetObjectItemCaseSensitive
 */
static inline cJSON *get_cJSON(const cJSON *json, struct json_attr attr)
{
    return cJSON_GetObjectItemCaseSensitive(json, attr.name);
}

#endif // LOADER_H
