#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include "utils.h"

static tmpfs_inode_t root_dir;

static void * tmpfs_init(struct fuse_conn_info *conn)
{
    // Initializes the file struct for the root directory.
    struct stat stat = {0};
    stat.st_uid = 1000;
    stat.st_gid = 1000;
    stat.st_mode = S_IFDIR | 0755;
    stat.st_nlink = 2;
    stat.st_size = 0;

    init_file(&root_dir, "/", &stat);

    return NULL;
}

static int tmpfs_getattr(const char * path, struct stat * statbuf)
{
    tmpfs_inode_t * file;

    int result = lookup(path, &root_dir, &file);

    if (0 == result)
    {
        memcpy(statbuf, &(file->stat), sizeof(*statbuf));
    }

    return result;
}

static int tmpfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
    tmpfs_inode_t * dir;

    int result = lookup(path, &root_dir, &dir);
    if (0 != result)
    {
        return result;
    }

    tmpfs_inode_t * dir_data = dir->data;
    size_t dir_len = dir->stat.st_size / sizeof(tmpfs_inode_t);

    for (int i = 0; i < dir_len; i++)
    {
        filler(buf, dir_data[i].name, &(dir_data[i].stat), 0);
    }

    filler(buf, ".", 0, 0);
    filler(buf, "..", 0, 0);

    return 0;
}

static int tmpfs_create(const char * path, mode_t mode, struct fuse_file_info * fi)
{
    struct stat stat = {0};
    tmpfs_inode_t * dir = NULL;
    tmpfs_inode_t file;
    char * path_copy_dir = NULL;
    char * path_copy_file = NULL;
    char * directoryname = NULL;
    char * filename = NULL;
    char * filename_copy = NULL;
    int result = 0;

    // Get dirname and filename.
    path_copy_dir = strdup(path);
    if (NULL == path_copy_dir)
    {
        result = -ENOMEM;
        goto cleanup;
    }
    path_copy_file = strdup(path);
    if (NULL == path_copy_file)
    {
        result = -ENOMEM;
        goto cleanup;
    }
    directoryname = dirname(path_copy_dir);
    filename = basename(path_copy_file);

    // Lookup dir.
    result = lookup(directoryname, &root_dir, &dir);
    if (0 != result)
    {
        goto cleanup;
    }
    
    // Init new file.
    filename_copy = strdup(filename);
    if (NULL == filename_copy)
    {
        result = -ENOMEM;
        goto cleanup;
    }
    stat.st_uid = 1000;
    stat.st_gid = 1000;
    stat.st_mode = S_IFREG | mode;
    stat.st_nlink = 1;
    stat.st_size = 0;
    init_file(&file, filename_copy, &stat);

    // Add the file to the directory.
    result = create_file(dir, &file);

cleanup:
    free(path_copy_dir);
    free(path_copy_file);
    if (0 != result)
    {
        free(filename_copy);
    }
    return result;
}

static int tmpfs_open(const char * path, struct fuse_file_info *fi)
{
    return 0;
}

static int tmpfs_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{
    tmpfs_inode_t * file;

    int result = lookup(path, &root_dir, &file);
    if (0 != result)
    {
        return result;
    }

    if (file->stat.st_mode & S_IFDIR)
    {
        return -EISDIR;
    }

    // If more bytes are requested then exist in the file, return as much as possible until the end of the file.
    if (file->stat.st_size - offset < size)
    {
        size = file->stat.st_size - offset;
    }

    memcpy(buf, file->data, size);

    return size;
}

static int tmpfs_utimens(const char * path, const struct timespec tv[2])
{
    tmpfs_inode_t * file;

    int result = lookup(path, &root_dir, &file);
    if (0 != result)
    {
        return result;
    }

    memcpy(&(file->stat.st_atim), tv, sizeof(*tv));
    memcpy(&(file->stat.st_mtim), tv + 1, sizeof(*tv));

    return 0;
}

static struct fuse_operations tmpfs_operations = {
    .getattr = tmpfs_getattr,
    .readdir = tmpfs_readdir,
    .init = tmpfs_init,
    .open = tmpfs_open,
    .create = tmpfs_create,
    .utimens = tmpfs_utimens,
    .read = tmpfs_read,
};

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    return fuse_main(args.argc, args.argv, &tmpfs_operations, NULL);
}
