#include "telegram.h"
#include "domfs.h"

int main(int argc, char **argv) {
    tg_initialize();
    fs_initialize();


    tg_close();

    return 0;
}