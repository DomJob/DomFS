#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    fs_mkdir("/directory");
    fs_create("/directory/qot.txt");

    fs_write("/directory/qot.txt", "Hello world et cetera", 0, 21);

    struct inode i;
    fs_getattr("/directory/qot.txt", &i);
    print_inode(&i);

    char buffer[2048];
    int n = fs_read("/directory/qot.txt", buffer, 0, 50);

    printf("Read %d chars: ");
    for(int i=0;i<n;i++)
        printf("%c", buffer[i]);
    printf("\n");
    
    fs_truncate("/directory/qot.txt", 15);

    fs_getattr("/directory/qot.txt", &i);
    print_inode(&i);

    n = fs_read("/directory/qot.txt", buffer, 0, 50);

    printf("Read %d chars: ");
    for(int i=0;i<n;i++)
        printf("%c", buffer[i]);
    printf("\n");
    


    disk_release();
    tg_close();
    return 0;
}

