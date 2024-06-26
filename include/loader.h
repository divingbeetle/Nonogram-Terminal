#ifndef LOADER_H
#define LOADER_H

#include <stdbool.h>
#include "cJSON/cJSON.h"

/**
 * @brief Expected spec for JSON field
 */
struct json_property
{
    const char *name; 
    int type;
    int min, max;
};

/**
 * @brief  Loads entire text file into memory
 * @retval NULL if error
 */
char *load_file(const char *file_name);

/**
 * @brief  Lists all JSON file names in a directory
 * @param  n_files_out Number of files found, output parameter
 * @return Array of strings, with directory path. NULL if error
 * @TODO   Portability improvements
 */
char **list_json_files(const char *dir_name, int *n_files_out);

/** 
 * @breif  Check if parsed json object has property with correct spec
 */
bool is_valid_json_property(const cJSON *json, const struct json_property prop);

/**
 * @brief  Parse JSON file into cJSON object
 * @retval NULL if error
 */
cJSON *cJSON_parse_file(const char *file_name);

/** 
 * @brief Wrapper for cJSON_GetObjectItemCaseSensitive
 */
static inline cJSON *get_cJSON(const cJSON *json, struct json_property prop)
{
    return cJSON_GetObjectItemCaseSensitive(json, prop.name);
}

#endif // LOADER_H
