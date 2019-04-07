#include "disk.h"

char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// Helper functions
char hex2byte(char first, char second);

BID seize_block() {
    // Todo
    return 0;
}

void read_block(BID id, char* data) {
    // Todo
}

void write_block(BID id, char* data) {
    // Todo
}

BID get_superblock() {
    // Todo
    return 0;
}

void set_superblock(BID id) {
    // Todo
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