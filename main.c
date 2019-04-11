#include "telegram.h"
#include "domfs.h"

int main(int argc, char **argv) {
    disk_initialize();
    tg_initialize();
    fs_initialize();
    
    fs_create("/hello.txt");
    fs_write("/hello.txt", "Hello World!", 0, 12);

    return 0;
}

