#pragma once

typedef struct {
    const char * name;
    struct stat stat;
    void * data;
} tmpfs_inode_t;

/**
 * Initializes a file struct with the given path as name and give stat.
 */
void init_file(tmpfs_inode_t * file, const char * path, const struct stat * stat);

/**
 * Performs a lookup for the file struct corresponding to the given path, starting from the given root dir.
 * Stores the result in the out parameter `file`.
 * Returns -ENOMEM or -ENOENT if some error occured, or 0 if successful.
 */
int lookup(const char * path, tmpfs_inode_t * root_dir, tmpfs_inode_t ** file);

/**
 * Looks for an entry with the given name inside the dir.
 * Returns the inode if found, else returns NULL.
 */
tmpfs_inode_t * dir_lookup(tmpfs_inode_t * dir, const char * name);

/**
 * Puts the given file in the dir.
 * Returns 0 iff successful, -ENOMEM or -EEXIST otherwise.
 */
int create_file(tmpfs_inode_t * dir, const tmpfs_inode_t * file);
