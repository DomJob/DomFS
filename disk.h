#ifndef DISK_H
#define DISK_H

int seize_block();

int read_block(int id, char* data);
int write_block(int id, char* data);

int get_superblock();
int set_superblock(int id);

#endif