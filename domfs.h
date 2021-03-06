#ifndef DOMFS_H
#define DOMFS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "macro.h"
#include "disk.h"

struct superblock {
    char check[11];
    BID address;
    BID root_inode;
    BID next_inode;
};

struct PACKED inode {
    BID block;
    uint64_t size   : 38;
    uint16_t nfiles : 14;
    uint16_t mode   : 6;
    uint8_t  offset : 6;

    uint32_t created;
    uint32_t modified;

    BID level1;
    BID level2;
    BID level3;
};

struct PACKED file {
    BID block;
    uint8_t offset;
    char name[256];
};

struct block_pointers {
    BID blocks[512];
};

struct inode_block {
    struct inode inodes[64];
};

// Called from main
int fs_initialize();

// Functions that FUSE will use
int fs_getattr(const char* path, struct inode* inode);
int fs_create(const char* path);
int fs_mkdir(const char* path);
int fs_write(const char* path, const char* buffer, int offset, int length);
int fs_read(const char* path, char* buffer, int offset, int length);
int fs_readdir(const char* path, struct file** listing);
int fs_chmod(const char* path, uint8_t new_mode);
int fs_hardlink(const char* source, const char* dest);
int fs_unlink(const char* path);
int fs_rmdir(const char* path);
int fs_rename(const char* source, const char* dest);
int fs_truncate(const char* path, uint64_t length);
int fs_format();

#endif