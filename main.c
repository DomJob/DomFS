#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode* inode);
long pack_listing(struct file* listing, int nbFiles, char* buffer);
int unpack_listing(struct file* listing, char* packed, long length, int nbFiles);

int main(int argc, char **argv) {
    /*
    tg_initialize();
    fs_initialize();
    tg_close();
    // */

    int n = 3;

    struct file listing[3];
    listing[0].name = ".";
    listing[0].block = 500;
    listing[0].offset = 3;
    listing[1].name = "..";
    listing[1].block = 555;
    listing[1].offset = 4;
    listing[2].name = "test.txt";
    listing[2].block = 566;
    listing[2].offset = 12;
/*
    char data[1000];
    int len = pack_listing(listing, n, data);
    printf("(%d) {", len);
    for(int i=0;i<len;i++)
        printf("0x%x, ", data[i] & 0xFF);
    printf("};\n");
/*/
    int len = 29;
    char data[29] = {0xf4, 0x1, 0x0, 0x0, 0x3, 0x2e, 0x3, 0x2b, 0x2, 0x0, 0x0, 0x4, 0x2e, 0x2e, 0x3, 0x36, 0x2, 0x0, 0x0, 0xc, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x3};

    struct file new_listing[4];
    unpack_listing(new_listing, data, len, 3);
    printf("\n");
    for(int i=0;i<n;i++) {
        printf("%d|%d - %s\n", new_listing[i].block, new_listing[i].offset, new_listing[i].name);
    }

    return 0;
}

