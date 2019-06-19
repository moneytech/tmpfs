#pragma once

typedef struct {
    const char * name;
    struct stat stat;
    void * data;
} tmpfs_file_t;

/**
 * Initializes a file struct with the given path as name and give stat.
 */
void init_file(tmpfs_file_t * file, const char * path, const struct stat * stat);

/**
 * Performs a lookup for the file struct corresponding to the given path, starting from the given root dir.
 * Stores the result in the out parameter `file`.
 * Returns -ENOMEM or -ENOENT if some error occured, or 0 if successful.
 */
int lookup(const char * path, tmpfs_file_t * root_dir, tmpfs_file_t ** file);
