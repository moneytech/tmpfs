#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utils.h"

void init_file(tmpfs_file_t * file, const char * path, const struct stat * stat)
{
    file->name = path;
    memcpy(&(file->stat), stat, sizeof(*stat));
    file->data = NULL;
}

int lookup(const char * path, tmpfs_file_t * root_dir, tmpfs_file_t ** file)
{
    tmpfs_file_t * dir = root_dir;
    int result = 0;
    char * entry = NULL;
    char * save_ptr = NULL;
    char * path_copy = strdup(path);
    if (NULL == path_copy)
    {
        result = -ENOMEM;
        goto cleanup;
    }

    // TODO relative paths.

    // Ensure the path starts with /
    if (*path != '/')
    {
        result = -ENOENT;
        goto cleanup; 
    }

    // Start iterating of the path entries and searching in the directories.
    entry = strtok_r(path_copy, "/", &save_ptr);
    while (NULL != entry)
    {
        // TODO move this to another function
        tmpfs_file_t * dir_entry = dir->data;
        size_t dir_len = dir->stat.st_size / sizeof(tmpfs_file_t);
        int found = 0;

        // Verify that it's a directory.
        if (!(dir->stat.st_mode & S_IFDIR))
        {
            result = -ENOENT;
            goto cleanup;
        }

        while (dir_entry < ((tmpfs_file_t *)dir->data) + dir_len)
        {
            // Check file name.
            if (0 == strcmp(dir_entry->name, entry))
            {
                dir = dir_entry;
                found = 1;
                break;
            }
            dir_entry++;
        }

        if (!found)
        {
            result = -ENOENT;
            goto cleanup;
        }

        entry = strtok_r(NULL, "/", &save_ptr);
    }

    *file = dir;
    
cleanup:
    free(path_copy);
    return result;
}
