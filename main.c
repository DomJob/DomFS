#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    fs_initialize();
   
    struct inode in;
    
    //int r = fs_create("/hello2.txt");
    //printf("Creation: %d\n", r);

    fs_getattr("/hello2.txt", &in);
    print_inode(&in);

    int k = fs_write("/hello2.txt", "ok what lmfao ect", 2045, 17);
    printf("Written: %d\n", k);

    fs_getattr("/hello2.txt", &in);
    print_inode(&in);

    tg_close();

    return 0;
}

