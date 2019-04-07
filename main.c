#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "telegram.h"

int main(int argc, char **argv) {
    tg_initialize();
    printf("Logged in!\n");


    int id = tg_send_message("hello");
    printf("Id: %u\n", id);

    return 0;
}