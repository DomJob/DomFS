#ifndef DOMFS_H
#define DOMFS_H

#include "macro.h"
#include "disk.h"

struct packed inode_address {
    BID block;
    uint8_t offset;
};

struct packed superblock {
    char check[10];
    BID address;
    struct inode_address root_inode;
    struct inode_address next_inode;
};

struct packed inode {
    struct inode_address address;
    uint64_t size;
    uint8_t nlinks;
    uint16_t mode;
    BID level0;
    BID level1;
    BID level2;
    BID level3;
};

struct packed dir_entry {
    struct inode_address inode;
    char name[256];
};

struct block_pointers {
    BID blocks[512];
};

struct inode_pointers {
    struct inode inodes[64];
};

// Called from main()
int fs_initialize();

// Functions that FUSE will use
int fs_getattr(char* path, struct inode* inode);
int fs_create(char* path);
int fs_mkdir(char* path);
int fs_read(char* path, char* buffer, long offset, long length);
int fs_readdir(char* path, struct dir_entry** entries);
int fs_write(char* path, char* buffer, long offset, long length);
int fs_chmod(char* path, uint16_t new_mode);
int fs_hardlink(char* source, char* dest);
int fs_unlink(char* path);
int fs_rmdir(char* path);
int fs_rename(char* source, char* dest);
int fs_truncate(char* path, long length);
int fs_format();

#endif