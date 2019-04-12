#include "telegram.h"
#include "domfs.h"

void show_stat(const char* path);
void list_directory(const char* path);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    show_stat("/deleteme");
    int k = fs_rmdir("/hardlink.txt");

    printf("rmdir said: %d\n", k);

    list_directory("/");
    disk_release();
    tg_close();
    return 0;
}

