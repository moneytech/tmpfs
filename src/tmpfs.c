#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>

typedef struct {
    const char * name;
    struct stat stat;
    void * data;
} tmpfs_file_t;

static tmpfs_file_t root_dir;

static void init_file(tmpfs_file_t * file, const char * path, const struct stat * stat)
{
    file->name = path;
    memcpy(&(file->stat), stat, sizeof(*stat));
    file->data = NULL;
}

static int tmpfs_lookup(const char * path, tmpfs_file_t ** file)
{
    tmpfs_file_t * dir = &root_dir;
    int result = 0;
    char * entry = NULL;
    char * save_ptr = NULL;
    char * path_copy = strdup(path);
    if (NULL == path_copy)
    {
        result = -ENOMEM;
        goto cleanup;
    }

    if (*path != '/')
    {
        // There was something before the initial /
        result = -ENOENT;
        goto cleanup; 
    }

    entry = strtok_r(path_copy, "/", &save_ptr);
    while (NULL != entry)
    {
        tmpfs_file_t * dir_entry = dir->data;
        size_t dir_len = dir->stat.st_size / sizeof(tmpfs_file_t);
        int found = 0;

        // Verify that it's a directory.
        if (!(dir->stat.st_mode & S_IFDIR))
        {
            result = -ENOENT;
            goto cleanup;
        }

        printf("dir name %s, dir_len %zu\n", dir->name, dir_len);

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

static void * tmpfs_init(struct fuse_conn_info *conn)
{
    printf("init called\n");
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
    printf("getattr called\n");
    tmpfs_file_t * file;

    int result = tmpfs_lookup(path, &file);

    if (0 == result)
    {
        memcpy(statbuf, &(file->stat), sizeof(*statbuf));
    }

    return result;
}

static int tmpfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
    tmpfs_file_t * dir;

    int result = tmpfs_lookup(path, &dir);
    if (0 != result)
    {
        return result;
    }

    tmpfs_file_t * dir_data = dir->data;
    size_t dir_len = dir->stat.st_size / sizeof(tmpfs_file_t);

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
    char * path_copy = NULL;
    char * filename = NULL;
    int result;
    tmpfs_file_t * file = NULL;
    struct stat stat = {0};

    // TODO lookup dirname
    tmpfs_file_t * dir = &root_dir;
    tmpfs_file_t * dir_data = (tmpfs_file_t *)dir->data;
    size_t dir_size = dir->stat.st_size;
    
    path_copy = strdup(path);
    if (NULL == path_copy)
    {
        result = -ENOMEM;
        goto cleanup;
    }

    filename = basename(path_copy);
    filename = strdup(filename);
    if (NULL == filename)
    {
        result = -ENOMEM;
        goto cleanup;
    }

    dir_data = realloc(dir_data, dir_size + sizeof(tmpfs_file_t));
    if (NULL == dir_data)
    {
        result = -ENOMEM;
        goto cleanup;
    }
    file = (tmpfs_file_t * )((char *)dir_data + dir_size);

    stat.st_uid = 1000;
    stat.st_gid = 1000;
    stat.st_mode = S_IFREG | mode;
    stat.st_nlink = 1;
    stat.st_size = 0;

    init_file(file, filename, &stat);

    dir->data = dir_data;
    dir->stat.st_size += sizeof(tmpfs_file_t);

    printf("dir data %p, dir size %zu\n", dir->data, dir->stat.st_size);
    printf("file name %s\n", file->name);
     
cleanup:
    free(path_copy);
    if (file->name != filename)
    {
        free(filename);
    }
    return result;
}

static int tmpfs_open(const char * path, struct fuse_file_info *fi)
{
    return 0;
}

static int tmpfs_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{
    tmpfs_file_t * file;

    int result = tmpfs_lookup(path, &file);
    if (0 != result)
    {
        return result;
    }

    if (file->stat.st_mode & S_IFDIR)
    {
        return -EISDIR;
    }

    if (file->stat.st_size - offset < size)
    {
        size = file->stat.st_size - offset;
    }

    memcpy(buf, file->data, size);

    return size;
}

static int tmpfs_utimens(const char * path, const struct timespec tv[2])
{
    tmpfs_file_t * file = &root_dir;

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
