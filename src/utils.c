#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "utils.h"

int add_inode_to_dir(tmpfs_inode_t *dir, const tmpfs_inode_t *inode)
{
    tmpfs_inode_t * dir_data = NULL;

    // TODO Ensure it's a dir? Is it even possible for it not to be a directory?

    // Validate that the dir doesn't have a file with the same name.
    if (NULL != dir_lookup(dir, inode->name))
    {
        return -EEXIST;
    }

    // Allocate space for the new file.
    dir_data = realloc(dir->data, dir->stat.st_size + sizeof(tmpfs_inode_t));
    if (NULL == dir_data)
    {
        return -ENOMEM;
    }

    // Copy the file into the new space.
    memcpy((char*)dir_data + dir->stat.st_size, inode, sizeof(tmpfs_inode_t));

    // Update the dir inode.
    dir->data = dir_data;
    dir->stat.st_size += sizeof(tmpfs_inode_t);

    return 0;
}

tmpfs_inode_t * dir_lookup(tmpfs_inode_t * dir, const char * name)
{
    tmpfs_inode_t * entries = (tmpfs_inode_t *)dir->data;
    size_t num_of_entries = dir->stat.st_size / sizeof(tmpfs_inode_t);

    // Ensure it's a directory.
    if (!(dir->stat.st_mode & S_IFDIR))
    {
        return NULL;
    }

    // Search for entry with the given name.
    for (int i=0; i < num_of_entries; i++)
    {
        if (0 == strcmp(entries[i].name, name))
        {
            return entries + i;
        }
    }

    return NULL;
}

void init_file(tmpfs_inode_t * file, const char * path, const struct stat * stat)
{
    file->name = path;
    memcpy(&(file->stat), stat, sizeof(*stat));
    file->data = NULL;
}

int lookup(const char * path, tmpfs_inode_t * root_dir, tmpfs_inode_t ** file)
{
    tmpfs_inode_t * dir = root_dir;
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

    // Start iterating over the path components and searching in the directories.
    entry = strtok_r(path_copy, "/", &save_ptr);
    while (NULL != entry)
    {
        tmpfs_inode_t * dir_entry = dir_lookup(dir, entry);

        if (NULL == dir_entry)
        {
            result = -ENOENT;
            goto cleanup;
        }

        dir = dir_entry;
        entry = strtok_r(NULL, "/", &save_ptr);
    }

    *file = dir;
    
cleanup:
    free(path_copy);
    return result;
}
