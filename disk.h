#ifndef DISK_H
#define DISK_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "macro.h"
#include "telegram.h"

BID seize_block();

void disk_initialize();
void disk_release();

void read_block(BID id, char* data);
void write_block(BID id, char* data, int length);
BID  get_superblock();
void set_superblock(BID id);

#endif