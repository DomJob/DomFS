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
    uint8_t name_length;
    char name[256];
};

struct block_pointers {
    BID blocks[512];
};

struct inode_pointers {
    struct inode inodes[64];
};


#endif