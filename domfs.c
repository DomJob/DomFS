#include "domfs.h"

BID superblock_id;

// Helper functions headers
struct inode fs_load_inode(struct inode_address addr);
BID fs_get_nth_block(long n, struct inode* inode);
int fs_update_inode(struct inode*);

// Load superblock, check if valid, format if not
int fs_initialize() {
    // Todo
    return 0;
}

// Helper function definitions