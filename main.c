#include <stdlib.h>
#include <stdio.h>

#include "domfs.h"
#include "disk.h"

int main() {
    initialize_client();


    td_json_client_destroy(client);
    return 0;
}