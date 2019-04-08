#ifndef DOMFS_H
#define DOMFS_H

#include "macro.h"
#include "disk.h"

struct superblock {
    char check[11];
    BID address;
    BID root_inode;
    BID next_inode;
};

struct packed inode {
    BID block;
    uint64_t size  : 38;
    uint16_t mode  : 12;
    uint8_t offset : 6;
    uint8_t nlinks;

    uint32_t created;
    uint32_t modified;

    BID level1;
    BID level2;
    BID level3;
};

struct file {
    BID block;
    uint8_t offset : 6;
    char* name;
};

struct block_pointers {
    BID blocks[512];
};

struct inode_pointers {
    struct inode inodes[64];
};

// Called from main()
int fs_initialize();

// Debug helpers
void print_inode(struct inode* inode);

// Functions that FUSE will use
int fs_getattr(char* path, struct inode* inode);
int fs_create(char* path);
int fs_mkdir(char* path);
int fs_read(char* path, char* buffer, long offset, long length);
int fs_readdir(char* path, struct file** entries);
int fs_write(char* path, char* buffer, long offset, long length);
int fs_chmod(char* path, uint16_t new_mode);
int fs_hardlink(char* source, char* dest);
int fs_unlink(char* path);
int fs_rmdir(char* path);
int fs_rename(char* source, char* dest);
int fs_truncate(char* path, long length);
int fs_format();

#endif