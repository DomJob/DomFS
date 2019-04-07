#include "telegram.h"
#include "disk.h"

int main(int argc, char **argv) {
    tg_initialize();

    BID id = 425; //seize_block();
    /*printf("Seized block %d!\n", id);
    printf("Writing in block...\n");
    write_block(id, "Hello everyone");*/
    printf("Reading block: ");
    char data[2096];
    read_block(id, data);
    printf("%s\n", data);
    
    tg_close();
    return 0;
}