#ifndef TELEGRAM_H
#define TELEGRAM_H

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

// td_log headers
void td_set_log_verbosity_level (int new_verbosity_level);
int td_set_log_file_path(const char *file_path);

// useful data and functions
void* client;
void tg_initialize();
struct tg_data_struct tg_data;
void tg_parse_data();

// Telegram interface that disk.c will use 

BID  tg_send_message(char* message);
int  tg_edit_message(BID id, char* message);
int  tg_read_message(BID id, char* message);
void tg_pin_message(BID id);
BID  tg_get_pinned_message();

#endif