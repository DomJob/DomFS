#include "disk.h"

char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
char hex2byte(char first, char second);

BID seize_block() {
    char* msg = "0";

    return tg_send_message(msg);
}

void read_block(BID id, char* data) {
    char hexdata[2*BLOCK_SIZE];

    for(int i=0; i<BLOCK_SIZE; i++) {
        data[i]        = '\0';
        hexdata[2*i]   = '0';
        hexdata[2*i+1] = '0';
    }

    tg_read_message(id, hexdata);

    for(int i=0; i<BLOCK_SIZE; i++) {
        char h1 = hexdata[2*i];
        char h2 = hexdata[2*i+1];

        data[i] = hex2byte(h1, h2);
    }
}

void write_block(BID id, char* data, int length) {
    char hexdata[2*length+1];

    for(int i=0; i<length; i++) {
        unsigned char byte = data[i];
        hexdata[2*i]   = hex[byte >> 4];
        hexdata[2*i+1] = hex[byte - (byte >> 4 << 4)];
    }

    // Trim right-most zeros 
    int hexlen;
    for(hexlen = 2*length; hexlen>1; hexlen--) {
        if(hexdata[hexlen] != '0') 
            break;
    }
    hexdata[hexlen] = '\0';
    tg_edit_message(id, hexdata);
}

BID get_superblock() {
    return tg_get_pinned_message();
}

void set_superblock(BID id) {
    tg_pin_message(id);
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