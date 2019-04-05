#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TIMEOUT 3.0

typedef struct {
    char api_id[100];
    char api_hash[100];
    char chat[100];
} tg_data_struct;

// td_json
void* td_json_client_create();
void td_json_client_send (void *client, const char *request);
char* td_json_client_receive (void *client, double timeout);
char* td_json_client_execute (void *client, const char *request);
void td_json_client_destroy (void *client);

// td_log
void td_set_log_verbosity_level (int new_verbosity_level);
int td_set_log_file_path(const char *file_path);

void* client;
tg_data_struct tg_data;

// Interface that disk.c will use 

unsigned int tg_send_message(char* message);
void tg_initialize();
void tg_parse_data();

#endif