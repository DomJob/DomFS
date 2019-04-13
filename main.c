#define FUSE_USE_VERSION 26

#include "macro.h"

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "telegram.h"
#include "disk.h"
#include "domfs.h"

static int df_getattr(const char* path, struct stat* stat) {
    DPRINT("getattr - %s\n", path);

    struct inode inode;
    if(fs_getattr(path, &inode) == -1)
        return -ENOENT;

    memset(stat, 0, sizeof(struct stat));

    if(inode.mode & M_IFDIR)
        stat->st_mode |= __S_IFDIR;
    if(inode.mode & M_IFREG)
        stat->st_mode |= __S_IFREG;
    if(inode.mode & M_IFLNK)
        stat->st_mode |= __S_IFLNK;
    
    if(inode.mode & M_PREAD)
        stat->st_mode |= S_IRUSR | S_IRGRP | S_IROTH;
    if(inode.mode & M_PWRITE)
        stat->st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;
    if(inode.mode & M_PEXEC)
        stat->st_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    
    stat->st_ino = (inode.block << 2) + inode.offset;
    stat->st_size = inode.size;
    stat->st_mtime = inode.modified;
    stat->st_ctime = inode.created;

    return 0;
}

static int df_create(const char* path , mode_t mode, struct fuse_file_info *finf) {
    DPRINT("create - %s\n", path);

    int res = fs_create(path);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EEXIST;
    if(res == -3) return -EPERM;
    if(res == -4) return -EPERM;

    return res;
}

static int df_mkdir(const char* path, mode_t mode) {
    DPRINT("mkdir - %s\n", path);

    int res = fs_mkdir(path);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EEXIST;
    if(res == -3) return -EPERM;
    if(res == -4) return -EPERM;

    return res;
}

static int df_write(const char* path, const char* buffer, size_t length, off_t offset, struct fuse_file_info *fo) {
    DPRINT("write %s\n", path);

    int res = fs_write(path, buffer, offset, length);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EPERM;
    if(res == -3) return -EISDIR;
    if(res == -4) return -EINVAL;

    return res;
}

static int df_read(const char* path, char* buffer, size_t length, off_t offset, struct fuse_file_info *fo) {
    DPRINT("read - %s\n", path);

    int res = fs_read(path, buffer, offset, length);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EPERM;
    if(res == -3) return -EISDIR;
    if(res == -4) return -EINVAL;

    return res;
}

static int df_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    DPRINT("readdir %s\n", path);

    struct file* listing;
    int nbFiles = fs_readdir(path, &listing);

    if(nbFiles == -1) return -ENOENT;
    if(nbFiles == -2) return -EPERM;
    if(nbFiles == -3) return -EPERM;

    for(int i=0; i<nbFiles; i++) {
        filler(buf, listing[i].name, NULL, 0);
    }
    free(listing);

    return 0;
}

static int df_chmod(const char* path, mode_t mode) {
    DPRINT("chmod %s\n", path);

    uint8_t newmode = 0;

    if(mode & S_IRUSR)
        newmode |= M_PREAD;
    if(mode & S_IWUSR)
        newmode |= M_PWRITE;
    if(mode & S_IXUSR)
        newmode |= M_PEXEC;

    int res = fs_chmod(path, newmode);
    if(res == -1) return -ENOENT;

    return 0;
}

static int df_hardlink(const char* source, const char* dest) {
    DPRINT("link %s to %s\n", source, dest);

    int res = fs_hardlink(source, dest);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EEXIST;
    if(res == -3) return -ENOENT;
    if(res == -4) return -EPERM;
    
    return 0;
}

static int df_unlink(const char* path) {
    DPRINT("unlink %s\n", path);

    int res = fs_unlink(path);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EISDIR;
    if(res == -3) return -EPERM;

    return 0;
}

static int df_rmdir(const char* path) {
    DPRINT("rmdir %s\n", path);

    int res = fs_rmdir(path);

    if(res == -1) return -ENOENT;
    if(res == -2) return -ENOTDIR;
    if(res == -3) return -ENOTEMPTY;
    if(res == -4) return -EPERM;

    return 0;
}

static int df_rename(const char* source, const char* dest){
    DPRINT("rename %s to %s\n", source, dest);

    int res = fs_rename(source, dest);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EEXIST;
    if(res == -3) return -ENOENT;
    if(res == -4) return -EPERM;
    

    return 0;
}

static int df_truncate(const char* path, off_t length) {
    DPRINT("truncate %s to %jd bytes\n", path, length);

    int res = fs_truncate(path, length);

    if(res == -1) return -ENOENT;
    if(res == -2) return -EISDIR;
    if(res == -3) return -EPERM;

    return 0;
}

static struct fuse_operations df_oper = {
    .getattr    = df_getattr,
    .readdir    = df_readdir,
    .create     = df_create,
    .mkdir      = df_mkdir,
    .write      = df_write,
    .read       = df_read,
    .chmod      = df_chmod,
    .link       = df_hardlink,
    .unlink     = df_unlink,
    .rmdir      = df_rmdir,
    .rename     = df_rename,
    .truncate   = df_truncate
};

int main(int argc, char* argv[]) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    int r = fuse_main(argc, argv, &df_oper, NULL);

    printf("Terminating all write procedures... Don't force-quit this!\n");
    disk_release();
    printf("Done. Closing down telegram...\n");
    tg_close();

    return r;
}

