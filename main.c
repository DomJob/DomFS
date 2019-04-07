#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "telegram.h"

int main(int argc, char **argv) {
    tg_initialize();
    printf("Logged in! Supergroup: %s\n", tg_data.supergroup);
    
    int id = tg_send_message("Hello world! new message");
    printf("Message sent, id = %d\n", id);
    char msg[4096];
    int r = tg_read_message(id, msg);

    printf("Message (%d): %s\n", r, msg);

    tg_pin_message(id);
    printf("Message pinned? :-)\n");



    return 0;
}