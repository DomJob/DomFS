#ifndef DISK_H
#define DISK_H

// Useful TDLib headers
void* td_json_client_create();
void td_json_client_send (void *client, const char *request);
char* td_json_client_receive (void *client, double timeout);
char* td_json_client_execute (void *client, const char *request);
void td_json_client_destroy (void *client);
void td_set_log_verbosity_level (int new_verbosity_level);
int td_set_log_file_path(const char *file_path);

// More telegram-related crap
void* client;

int initialize_client();


// Disk-related functions
int seize_block();

int read_block(int id, char* data);
int write_block(int id, char* data);

int get_superblock();
int set_superblock(int id);

#endif