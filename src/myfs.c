#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <string.h>
#include <errno.h>

const char * filename = "siski";
const char * content = "sie sind toll\n";

static int myfs_getattr(const char * path, struct stat * statbuf)
{
    memset(statbuf, 0, sizeof(*statbuf));
    statbuf->st_uid = 1000;
    statbuf->st_gid = 1000;

    if (0 == strcmp(path, "/"))
    {
      statbuf->st_mode = S_IFDIR | 0755;
      statbuf->st_nlink = 2;
    }
    else if (0 == strcmp(path+1, filename))
    {
        statbuf->st_mode = S_IFREG | 0444;
        statbuf->st_nlink = 1;
        statbuf->st_size = strlen(content);
    }
    else
    {
        return -ENOENT;
    }


    return 0;
}

static int myfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi)
{
    filler(buf, ".", 0, 0);
    filler(buf, "..", 0, 0);
    filler(buf, filename, 0, 0);

    return 0;
}

static int myfs_open(const char * path, struct fuse_file_info *fi)
{
    if (0 != strcmp(path+1, filename))
    {
        return -ENOENT;
    }
    if ((fi->flags & O_ACCMODE) != O_RDONLY)
    {
        return -EACCES;
    }
    
    return 0;
}

static int myfs_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * fi)
{
    size_t len = 0;
    if (0 != strcmp(path+1, filename))
    {
        return -ENOENT;
    }

    len = strlen(content);
    if (offset < len)
    {
        if (offset + size > len)
        {
            size = len - offset;
        }
        memcpy(buf, content + offset, size);
    }
    else
    {
        size = 0;
    }

    return size;
}

static struct fuse_operations myfs_operations = {
    .getattr = myfs_getattr,
    .readdir = myfs_readdir,
    .open = myfs_open,
    .read = myfs_read,
};

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    return fuse_main(args.argc, args.argv, &myfs_operations, NULL);
}
