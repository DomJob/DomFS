#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include "telegram.h"

#define BLOCK_SIZE 2096
#define INODE_SIZE 32
#define BID uint32_t

BID seize_block();

void read_block(BID id, char* data);
void write_block(BID id, char* data);

BID get_superblock();
void set_superblock(BID id);

#endif