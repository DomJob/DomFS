#ifndef DOMFS_H
#define DOMFS_H

#include "disk.h"

#define packed __attribute__((__packed__)) 

typedef struct {
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint8_t b4;
    uint8_t b5;
} uint40_t;

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
    uint40_t size;
    uint32_t created;
    uint32_t modified;
    uint8_t nlinks;
    uint8_t mode;
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