#include "disk.h"


int initialize_client() {
    td_set_log_verbosity_level(0);
    td_set_log_file_path("./client.log");

    client = td_json_client_create();

    

   
}