#include "disk.h"

char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// Helper functions
char hex2byte(char first, char second);

// Disk emulator for testing without overloading Telegram with dumb stuff

typedef struct {
    char data[2*BLOCK_SIZE];
} block;

block blocks[500];

BID superblock = 0;
BID nextfree = 0;

BID seize_block() {
    for(int i=0; i<2*BLOCK_SIZE; i++)
        blocks[nextfree].data[i] = '0';
    return nextfree++;
}

void read_block(BID id, char* data) {
    for(int i=0; i<BLOCK_SIZE; i++) {
        char first  = blocks[id].data[2*i];
        char second = blocks[id].data[2*i+1];

        data[i] = hex2byte(first, second);
    }
}

void write_block(BID id, char* data) {
    for(int i=0; i<BLOCK_SIZE; i++) {
        unsigned char byte = data[i];
        blocks[id].data[2*i]   = hex[byte >> 4];
        blocks[id].data[2*i+1] = hex[byte - (byte >> 4 << 4)];
    }
}

BID get_superblock() {
    return superblock;
}

void set_superblock(BID id) {
    superblock = id;
}

char hex2byte(char first, char second) {
    if(97 <= first)
        first -= 39;
    if(97 <= second)
        second -= 39;
    if(65 <= first)
        first -= 7;
    if(65 <= second)
        second -= 7;

    first -= 48;
    second -= 48;
    char byte = (first << 4) + second;
    return byte;
}