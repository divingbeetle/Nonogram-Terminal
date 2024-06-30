#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "loader.h"
#include "utils.h"

/* Function Prototypes */

long get_file_size(FILE *fp);
DIR *open_directory(const char *dir_name);

struct dirent *next_json_entry(DIR *dir);
char *construct_file_path(const char *dir_name, const char *file_name);

bool is_valid_property_type(const cJSON *obj, struct json_property prop);
bool is_valid_property_range(const cJSON *obj, struct json_property prop);

/* Public */

bool file_exists(const char *file_name)
{
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) return false;
    fclose(fp);
    return true;
}

char *load_file(const char *file_name)
{
    FILE *fp  = NULL;
    char *buf = NULL;

    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        LOGF(LOG_ERROR, "Failed to open file: '%s'", file_name);
        return NULL;
    }

    long file_size = get_file_size(fp);
    if (file_size <= 0)
    {
        LOGF(LOG_ERROR, "Failed to get valid file size: '%s'", file_name);
        goto cleanup;
    }

    buf = malloc(file_size + 1);
    if (buf == NULL)
    {
        LOG(LOG_ERROR, "Memory allocation failed");
        goto cleanup;
    }

    if (fread(buf, 1, file_size, fp) != file_size)
    {
        LOGF(LOG_ERROR, "Failed to read file: %s", file_name);
        goto cleanup;
    }

    fclose(fp);
    buf[file_size] = '\0';
    return buf;

cleanup:
    if (fp != NULL)  fclose(fp);
    if (buf != NULL) free(buf);
    return NULL;
}

char **list_json_files(const char *dir_name, int *n_files_out)
{
    assert(dir_name != NULL);
    assert(n_files_out != NULL);

    DIR *dir = NULL;
    char **file_list = NULL;
    int files_cnt = 0;

    dir = open_directory(dir_name);
    if (dir == NULL) return NULL;

    // Count numbers first to allocate memory
    while (next_json_entry(dir) != NULL)
    {
        files_cnt++;
    }

    file_list = malloc(files_cnt * sizeof(*file_list));
    if (file_list == NULL)
    {
        LOG(LOG_ERROR, "Memory allocation failed");
        goto cleanup;
    }

    // Reset
    rewinddir(dir);
    files_cnt = 0;

    // Store 
    struct dirent *entry;
    while ((entry = next_json_entry(dir)) != NULL)
    {
        char *file_path = construct_file_path(dir_name, entry->d_name);
        if (file_path == NULL)
        {
            goto cleanup;
        }
        file_list[files_cnt++] = file_path;
    }

    closedir(dir);
    *n_files_out = files_cnt;
    return file_list;

cleanup:
    if (dir != NULL)       closedir(dir);
    if (file_list != NULL) free_ptr_array((void **) file_list, files_cnt);
    return NULL;
}

bool is_valid_json_property(const cJSON *json, const struct json_property prop)
{
    cJSON *obj = get_cJSON(json, prop);

    return (is_valid_property_type(obj, prop) &&
            is_valid_property_range(obj, prop));
}

cJSON *cJSON_parse_file(const char *file_name)
{
    char *file_buf = load_file(file_name);
    if (file_buf == NULL) return NULL;

    const char *error_ptr = NULL;
    cJSON *json = cJSON_Parse(file_buf);
    if (json == NULL)
    {
        error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOGF(LOG_WARNING, 
                 "cJSON Error parsing '%s' at %s", file_name, error_ptr);
        }
        else
        {
            LOGF(LOG_WARNING, "cJSON Error parsing '%s'", file_name);
        }
    }

    free(file_buf);
    return json;
}

/* Private */

long get_file_size(FILE *fp)
{
    assert(fp != NULL);

    long size;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    return size;
}

DIR *open_directory(const char *dir_name)
{
    DIR *dir = opendir(dir_name);
    if (dir == NULL)
    {
        switch (errno)
        {
            case ENOENT:
                LOGF(LOG_ERROR, "Directory not found: '%s'", dir_name);
                break;
            case EACCES:
                LOGF(LOG_ERROR, "Access denied to directory: '%s'", dir_name);
                break;
            case ENOTDIR:
                LOGF(LOG_ERROR, "Not a directory: '%s'", dir_name);
                break;
            default:
                LOGF(LOG_ERROR, "Failed to open directory: '%s'", dir_name);
                break;
        }
    }
    return dir;
}

struct dirent *next_json_entry(DIR *dir)
{
    assert(dir != NULL);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN)
        {
            char *ext = strrchr(entry->d_name, '.');
            if (ext != NULL && strcmp(ext, ".json") == 0)
            {
                return entry;
            }
        }
    }
    return NULL;
}

char *construct_file_path(const char *dir_name, const char *file_name)
{
    assert(file_name != NULL);
    assert(dir_name != NULL);

    int   path_len  = strlen(dir_name) + strlen(file_name) + 2;
    char *full_path = malloc(path_len);
    ALLOC_CHECK_RETURN(full_path, NULL);

    snprintf(full_path, path_len, "%s/%s", dir_name, file_name);
    return full_path;
}

bool is_valid_property_type(const cJSON *obj, const struct json_property prop)
{
    switch (prop.type)
    {
        case cJSON_String:
            if (cJSON_IsString(obj) && obj->valuestring)
                return true;
            break;
        case cJSON_Number:
            if (cJSON_IsNumber(obj))
                return true;
            break;
        case cJSON_Array:
            if (cJSON_IsArray(obj))
                return true;
            break;
        default:
            // @TODO Add more types when needed
            return false;
            break;
    }

    LOGF(LOG_WARNING,
        "Invalid or missing JSON property: '%s'", prop.name);
    return false;
}

bool is_valid_property_range(const cJSON *obj, const struct json_property prop)
{
    int         check_val;
    const char *check_type;

    switch (prop.type)
    {
        case cJSON_String:
            check_val  = strlen(obj->valuestring);
            check_type = "text length";
            break;
        case cJSON_Number:
            check_val  = obj->valueint;
            check_type = "value";
            break;
        case cJSON_Array:
            check_val  = cJSON_GetArraySize(obj);
            check_type = "array size";
            break;
        default:
            // @TODO Add more types when needed
            return false;
            break;

    }

    if (check_val < prop.min || check_val > prop.max)
    {
        LOGF(LOG_WARNING, 
             "Invalid range for JSON attribute '%s' %s: "
             "Got %d, Expected %d to %d", 
             prop.name, check_type, check_val, prop.min, prop.max);

        return false;
    }
    return true;
}

