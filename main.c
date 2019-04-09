#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode* inode);
int update_inode(struct inode* inode);

int main(int argc, char **argv) {
    tg_initialize();
    fs_initialize();

    struct inode search;
    int ret = fs_getattr("/.././.././.././.././././..",&search);
    printf("\n\n---\n Return: %d\n", ret);
    print_inode(&search);

    tg_close();

    return 0;
}