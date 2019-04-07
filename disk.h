#ifndef DISK_H
#define DISK_H

#include "macro.h"
#include "telegram.h"

BID seize_block();

void read_block(BID id, char* data);
void write_block(BID id, char* data);
BID  get_superblock();
void set_superblock(BID id);

#endif