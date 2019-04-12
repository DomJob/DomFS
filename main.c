#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    struct inode i;
    fs_getattr("/directory", &i);
    print_inode(&i);


    disk_release();
    tg_close();
    return 0;
}

