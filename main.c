#include "telegram.h"
#include "domfs.h"

void print_inode(struct inode*);

int main(int argc, char **argv) {
    tg_initialize();
    disk_initialize();
    fs_initialize();

    /*    
    fs_create("/hello.txt");
    char numbers[] = {'0','1','2','3','4','5','6','7','8','9'};
    char string[2500];
    for(int i=0; i<2500; i++)
        string[i] = numbers[i%10];
    
    fs_write("/hello.txt", string, 0, 2500);
    */
    char ect[2048];
    int n = fs_read("/hello.txt", ect, 2495, 10);

    printf("Read %d bytes:", n);
    for(int i=0; i<n; i++)
        printf("%c", ect[i]);
    printf("\n");

    disk_release();
    tg_close();
    return 0;
}

