#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    fs_create("/hello.txt");

    struct inode i;
    fs_getattr("/", &i);
    print_inode(&i);
/*
    struct file* listing;

    int n = fs_readdir("/", &listing);
    printf("Got %d entries\n", n);
    for(int i=0;i<n;i++)
        printf("%s\n", listing[i].name);
    
    free(listing);
*/

    disk_release();
    tg_close();
    return 0;
}

