#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <pthread.h>

#include "macro.h"

struct tg_data_struct {
    char api_id[100];
    char api_hash[100];
    char chat[100];
    char supergroup[100];
};

// td_json headers
void* td_json_client_create();
void  td_json_client_send(void *client, const char *request);
char* td_json_client_receive(void *client, double timeout);
void td_json_client_destroy (void *client);

// td_log headers
void td_set_log_verbosity_level (int new_verbosity_level);
int td_set_log_file_path(const char *file_path);

// useful data and functions
void* client;
void tg_initialize();
void tg_close();
struct tg_data_struct tg_data;

// Telegram interface that disk.c will use 
BID  tg_send_message(char* message);
int  tg_edit_message(BID id, char* message);
int  tg_read_message(BID id, char* message);
void tg_pin_message(BID id);
BID  tg_get_pinned_message();
void tg_delete_message(BID id);

#endif