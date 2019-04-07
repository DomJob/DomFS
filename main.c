#include "domfs.h"
#include "telegram.h"

int main(int argc, char **argv) {
    tg_initialize();

    BID id = tg_send_message("Hello");
    tg_pin_message(id);

    return 0;
}