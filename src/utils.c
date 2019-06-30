#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include "utils.h"

/**
 * A safe basename implementation that doesn't modify the path. Taken from Glibc.
 * @return the filenmae.
 */
static const char * safe_basename(const char * path)
{
    char * p = strrchr (path, '/');
    return p ? p + 1 : (char *) path;
}

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

int init_file(tmpfs_inode_t *file, const char *name, const struct stat *stat)
{
    file->name = strdup(name);
    if (NULL == file->name)
    {
        return -ENOMEM;
    }
    memcpy(&(file->stat), stat, sizeof(*stat));
    file->data = NULL;
    return 0;
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

int split(const char * path, const tmpfs_inode_t * root_dir, tmpfs_inode_t ** dir, char ** filename)
{
    char * path_copy= NULL;
    char * directory_name = NULL;
    int result = 0;

    // Get the directory name.
    path_copy = strdup(path);
    if (NULL == path_copy)
    {
        result = -ENOMEM;
        goto cleanup;
    }
    directory_name = dirname(path_copy);

    // Get the directory inode.
    result = lookup(directory_name, root_dir, dir);
    if (0 != result)
    {
        goto cleanup;
    }

    // Get the file name.
    *filename = safe_basename(path);

    result = 0;

cleanup:
    free(path_copy);
    return result;
}
