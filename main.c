#include "telegram.h"
#include "domfs.h"

int main(int argc, char **argv) {
    tg_initialize();
    fs_initialize();

    fs_create("/test5.txt");
    fs_create("/test4.txt");
    fs_create("/test1.txt");
    fs_create("/test2.txt");
    fs_create("/test3.txt");

    tg_close();

    return 0;
}

