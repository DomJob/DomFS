#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();
    

    
    fs_create("/hello.txt");
    fs_write("/hello.txt", "Hello World!", 0, 12);
    

    disk_release();
    tg_close();
    return 0;
}

