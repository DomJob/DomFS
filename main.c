#include "telegram.h"
#include "domfs.h"

void show_stat(const char* path);
void list_directory(const char* path);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    list_directory("/");

    fs_rename("/renamed.txt", "/ect.txt");

    list_directory("/");
    disk_release();
    tg_close();
    return 0;
}

