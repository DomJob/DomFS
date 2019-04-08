#include "domfs.h"

struct superblock superblock;

// Helper functions headers
struct inode get_inode(struct inode_address address);
BID get_nth_block(long n, struct inode* inode);
int update_inode(struct inode* inode);
int update_superblock();
int pack_listing(struct file* listing, int nbFiles, char* buffer);

// Load superblock, format if superblock isn't valid or if no message is pinned
int fs_initialize() {
    printf("Initializing DomFS...\n");
    BID sb_id = get_superblock();

    if(sb_id == 0) {
        printf("No pinned message found. Formatting...\n");
        fs_format();
        return 1;
    }

    char sb_data[2048];
    read_block(sb_id, sb_data);

    struct superblock *sb = (struct superblock*) sb_data;

    if(strcmp(sb->check, "SUPERBLOCK") != 0) {
        printf("Superblock is invalid. Formatting...\n");
        fs_format();
        return 2;
    }

    printf("Everything is correct!\n");

    superblock.address = sb->address;
    superblock.root_inode = sb->root_inode;
    superblock.next_inode = sb->next_inode;
    strcpy(superblock.check, sb->check);

    return 0;
}

int fs_format() {
    BID sb_id = seize_block();
    BID root_inode_block = seize_block();
    BID root_inode_data  = seize_block();

    // Set up superblock
    superblock.address = sb_id;
    superblock.root_inode.block  = root_inode_block;
    superblock.root_inode.offset = 0;
    superblock.next_inode.block  = root_inode_block;
    superblock.next_inode.offset = 1;
    strcpy(superblock.check, "SUPERBLOCK");
    update_superblock();
    set_superblock(sb_id);

    // Set up root data directory
    struct file listing[2];
    listing[0].address = superblock.root_inode;
    listing[0].name    = ".";
    listing[1].address = superblock.root_inode;
    listing[1].name    = "..";
    
    char data[2048];
    int len = pack_listing(listing, 2, data);
    write_block(root_inode_data, data, len);

    // Set up root inode
    struct inode root_inode;
    root_inode.address.block  = root_inode_block;
    root_inode.address.offset = 0;
    root_inode.size   = len;
    root_inode.nlinks = 2;
    root_inode.mode   = G_IFDIR | 0666;
    root_inode.level0 = root_inode_data;
    root_inode.level1 = 0;
    root_inode.level2 = 0;
    root_inode.level3 = 0;

    update_inode(&root_inode);
}

// Helper function definitions

int update_superblock() {
    char* sb_data = (char*) &superblock;
    write_block(superblock.address, sb_data, sizeof(struct superblock));

    return 0;
}

int update_inode(struct inode* inode) {
    char data[BLOCK_SIZE];
    read_block(inode->address.block, data);

    char* bin_inode = (char*) inode;
    int offset = INODE_SIZE * inode->address.offset;
    for(int i=0; i<INODE_SIZE; i++) {
        data[offset+i] = bin_inode[i];
    }

    write_block(inode->address.block, data, 2048);

    return 0;
}

int pack_listing(struct file* listing, int nbFiles, char* buffer) {
    int c=0;

    for(int i=0; i<nbFiles; i++) {
        char* addr_bin = (char*) &listing[i].address;
        for(int j=0; j<5; j++)
            buffer[c++] = addr_bin[j];
        int len = strlen(listing[i].name);
        for(int j=0; j<len; j++)
            buffer[c++] = listing[i].name[j];
        buffer[c++] = 0x03;
    }

    return c;
}