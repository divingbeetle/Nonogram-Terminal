#include "loader.h"
#include "utils.h"
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

/* Function Prototypes */

long get_file_size(FILE *fp);
DIR *open_directory(const char *dir_name);
struct dirent *get_next_json_file(DIR *dir);
char *create_file_path(const char *dir_name, const char *file_name);

bool validate_type(const cJSON *obj, 
                   struct json_attr attr, const char *file_name);
bool validate_range(const cJSON *obj, 
                    struct json_attr attr, const char *file_name);
/* Public */

char *load_file(const char *file_name)
{
    assert(file_name != NULL);
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        LOGF(LOG_ERROR, "Failed to open file: %s", file_name);
        return NULL;
    }

    long file_size = get_file_size(fp);
    char *buf      = malloc(file_size + 1);
    if (buf == NULL)
    {
        LOG(LOG_ERROR, "Memory allocation failed");
        fclose(fp);
        return NULL;
    }
    fread(buf, 1, file_size, fp);
    buf[file_size] = '\0';
    fclose(fp);

    return buf;
}

char **get_json_files_list(const char *dir_name, int *n_files_out)
{
    DIR *dir = open_directory(dir_name);
    if (dir == NULL)
    {
        return NULL;
    }

    int n_files = 0;
    while (get_next_json_file(dir) != NULL)
    {
        n_files++;
    }

    char **files_list = malloc(n_files * sizeof(*files_list));
    if (files_list == NULL)
    {
        LOG(LOG_ERROR, "Memory allocation failed");
        closedir(dir);
        return NULL;
    }

    rewinddir(dir);
    n_files = 0;

    struct dirent *entry;
    while ((entry = get_next_json_file(dir)) != NULL)
    {
        char *file_path = create_file_path(dir_name, entry->d_name);
        if (file_path == NULL)
        {
            LOG(LOG_ERROR, "Memory allocation failed");
            free_ptr_array((void **) files_list, n_files);
            closedir(dir);
            return NULL;
        }
        files_list[n_files++] = file_path;
    }

    closedir(dir);
    *n_files_out = n_files;
    return files_list;
}

bool validate_attr(const cJSON *json, 
                   struct json_attr attr, const char *file_name)
{
    assert(json != NULL);
    assert(file_name != NULL);

    cJSON *obj = get_cJSON(json, attr);

    if (!validate_type(obj, attr, file_name))
        return false;
    if (!validate_range(obj, attr, file_name))
        return false;

    return true;
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
    assert(dir_name != NULL);

    DIR *dir = opendir(dir_name);
    if (dir == NULL)
    {
        switch (errno)
        {
            case ENOENT:
                LOGF(LOG_ERROR, "Directory does not exist: %s", dir_name);
                break;
            case EACCES:
                LOGF(LOG_ERROR, "Permission denied: %s", dir_name);
                break;
            case ENOTDIR:
                LOGF(LOG_ERROR, "Not a directory: %s", dir_name);
                break;
            default:
                LOGF(LOG_ERROR, "Failed to open directory: %s", dir_name);
                break;
        }
        return NULL;
    }
    return dir;
}

struct dirent *get_next_json_file(DIR *dir)
{
    assert(dir != NULL);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
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

char *create_file_path(const char *dir_name, const char *file_name)
{
    assert(file_name != NULL);
    assert(dir_name != NULL);

    int path_len    = strlen(dir_name) + strlen(file_name) + 2;
    char *full_path = malloc(path_len);
    ALLOC_CHECK_RETURN(full_path, NULL);

    snprintf(full_path, path_len, "%s/%s", dir_name, file_name);
    return full_path;
}

bool validate_type(const cJSON *obj, 
                   struct json_attr attr, const char *file_name)
{
    assert(file_name != NULL);

    switch (attr.type)
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
            return false;
            break;
    }

    LOGF(LOG_WARNING,
        "%s - Invalid or missing JSON attribute: %s", file_name, attr.name);
    return false;
}

bool validate_range(const cJSON *obj, 
                    struct json_attr attr, const char *file_name)
{
    assert(file_name != NULL);
    
    int val;
    if (cJSON_IsNumber(obj))
    {
        val = obj->valueint;
    }
    else if (cJSON_IsString(obj))
    {
        val = strlen(obj->valuestring);
    }
    else if (cJSON_IsArray(obj))
    {
        val = cJSON_GetArraySize(obj);
    }
    else 
    {
        return false;
    }

    if (val < attr.min || val > attr.max)
    {
        LOGF(LOG_WARNING, 
             "%s - Invalid JSON attribute %s: Got %d, Expected %d to %d", 
             file_name, attr.name, val, attr.min, attr.max);

        return false;
    }
    return true;
}

