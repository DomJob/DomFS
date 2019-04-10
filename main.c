#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    fs_initialize();
   
    struct inode i;
    int r = fs_create("/hello.txt");
    printf("Creation: %d\n", r);
    
    fs_getattr("/hello.txt", &i);
    print_inode(&i);

    for(int i=0;i<2040;i+=10) {
        int k = fs_write("/hello.txt", "123456789 ", i, 10);
        printf("Written: %d\n", k);
    }

    fs_getattr("/hello.txt", &i);
    print_inode(&i);

    tg_close();

    return 0;
}

