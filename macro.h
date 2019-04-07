#ifndef MACRO_H
#define MACRO_H

#include <stdint.h>

#define packed          __attribute__((__packed__)) 

#define TIMEOUT         3.0
#define BLOCK_SIZE      2048
#define INODE_SIZE      32
#define BID             uint32_t

#define MAX_FILE_SIZE   275415828480 // (1 + 512 + 512^2 + 512^3) * 2048 bytes

#endif