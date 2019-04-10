#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    fs_initialize();

    fs_mkdir("/test3");
    fs_create("/test3/a2.txt");
    fs_mkdir("/test3/test");
    fs_create("/test3/test/k.txt");

    struct inode i;
    int r = fs_getattr("/test3/test/k.txt", &i);
    printf("r: %d\n", r);
    if(r == 0) {
        print_inode(&i);
    }
    tg_close();

    return 0;
}

