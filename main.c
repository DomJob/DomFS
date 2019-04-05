#include <stdlib.h>
#include <stdio.h>

#include "domfs.h"
#include "disk.h"
#include "telegram.h"

int main(int argc, char **argv) {
    tg_initialize();

    int res = tg_send_message("Hello world");
    printf("%d\n", res);

    return 0;
}