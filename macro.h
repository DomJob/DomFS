#ifndef MACRO_H
#define MACRO_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define packed          __attribute__((__packed__)) 

#define TIMEOUT         1.0
#define BLOCK_SIZE      2048
#define INODE_SIZE      32
#define BLOCK_ADDR      5
#define BID             uint32_t

#define MAX_FILE_SIZE   275415828480 // (1 + 512 + 512^2 + 512^3) * 2048 bytes

#define G_IFDIR 0b100000000000
#define G_IFREG 0b010000000000
#define G_IFLNK 0b001000000000

#define G_IRUSR 0b000100000000
#define G_IWUSR 0b000010000000
#define G_IXUSR 0b000001000000
#define G_IRWXU 0b000111000000

#define G_IRGRP 0b000000100000
#define G_IWGRP 0b000000010000
#define G_IXGRP 0b000000001000
#define G_IRWXG 0b000000111000

#define G_IRALL 0b000000000100
#define G_IWALL 0b000000000010
#define G_IXALL 0b000000000001
#define G_IRWXA 0b000000000111


#endif