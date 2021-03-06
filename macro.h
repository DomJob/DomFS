#ifndef MACRO_H
#define MACRO_H


#define PACKED          __attribute__((__packed__)) 

#define DEBUG

#define TIMEOUT         3.0
#define BLOCK_SIZE      2048
#define INODE_SIZE      32
#define BLOCK_ADDR      5
#define MAX_NAME_SIZE   256
#define MAX_PATH_SIZE   4096
#define BID             uint32_t

#define MAX_FILE_SIZE   275415826432 // (512 + 512^2 + 512^3) * 2048 bytes

#define M_IFDIR  0b100000
#define M_IFREG  0b010000
#define M_IFLNK  0b001000

#define M_PREAD  0b000100
#define M_PWRITE 0b000010
#define M_PEXEC  0b000001
#define M_PALL   0b000111


#ifdef DEBUG
#define DPRINT(...) printf( __VA_ARGS__ );
#else
#define DPRINT(...) do{ } while ( 0 );
#endif



#endif