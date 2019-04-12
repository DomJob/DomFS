#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);
void list_directory(const char* path);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    list_directory("/");/*
    list_directory("/directory");

    fs_hardlink("/directory/whatever.txt", "/hardlink.txt");

    list_directory("/");
    fs_write("/hardlink.txt", "Hello world", 0, 11);
    list_directory("/directory");*/

    disk_release();
    tg_close();
    return 0;
}

