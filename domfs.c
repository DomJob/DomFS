#include "domfs.h"

struct superblock superblock;

// Helper functions headers
struct inode get_inode(BID block, uint8_t offset);
BID get_nth_block(long n, struct inode* inode);
int update_inode(struct inode* inode);
int update_superblock();
int pack_listing(struct file* listing, int nbFiles, char* buffer);
int seize_inode(struct inode* inode);
int get_filename(const char* path, char *filename);
int get_parent(const char* path, char* parent);

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

// Gets inode from path
// Returns:
//   0 : Success
//  -1 : Not found
int fs_getattr(const char* path, struct inode* inode) {
    BID block;
    uint8_t offset;

    if(strcmp(path, "/") == 0) {
        // It's the root inode
        block = superblock.root_inode;
        offset = 0;
    } else {
        // Find the parent's inode
        char parent[2048];
        char filename[256];
        get_parent(path, parent);
        get_filename(path, filename);

        struct inode parent_inode;
        if(fs_getattr(parent, &parent_inode) == -1)
            return -1;

        // Find the file's inode from its entries
        char tmpname[256];
        int tmpname_pos = -2;

        char block_bin[4];
        uint8_t block_bin_pos = 0;
        
        char data[2048];
        int found = 0;
        for(int blockno = 0; blockno < parent_inode.size / 2048 + 1; blockno++) {
            int addr = get_nth_block(blockno, &parent_inode);
            read_block(addr, data);

            for(int i=0; i<2048; i++) {
                if(tmpname_pos == -2) {
                    // Reading inode block (4 bytes)
                    block_bin[block_bin_pos++] = data[i];

                    if(block_bin_pos == 4) {
                        block_bin_pos = 0;
                        tmpname_pos = -1;
                        block = *((uint16_t*) block_bin);
                        if(block == 0) // Invalid block id, thus end of listing reached
                            break;
                    }
                } else if(tmpname_pos == -1) {
                    // Reading offset (1 byte)
                    offset = data[i];
                    tmpname_pos = 0;
                } else {
                    // Reading file name (1-255 bytes)
                    char byte = data[i];
                    if(byte == 0x03) { // End of file name reached
                        tmpname[tmpname_pos] = 0x00;
                        if(strcmp(tmpname, filename) == 0) {
                            // File name found
                            found = 1;
                            break;
                        }
                        tmpname_pos = -2;
                    } else {
                        tmpname[tmpname_pos++] = byte;
                    }
                }
            }
        }

        if(!found)
            return -1;
    }

    struct inode file_inode = get_inode(block, offset);
    inode->block    = file_inode.block;
    inode->offset   = file_inode.offset;
    inode->size     = file_inode.size;
    inode->mode     = file_inode.mode;
    inode->nlinks   = file_inode.nlinks;
    inode->created  = file_inode.created;
    inode->modified = file_inode.modified;
    inode->level1   = file_inode.level1;
    inode->level2   = file_inode.level2;
    inode->level3   = file_inode.level3;

    return 0;
}

// Formats the "disk" aka posts and pins a new superblock and initializes the root directory
int fs_format() {
    BID sb_id = seize_block();
    BID root_inode_block = seize_block();
    BID root_inode_lvl1  = seize_block();
    BID root_inode_data  = seize_block();

    // Set up superblock
    superblock.address = sb_id;
    superblock.root_inode  = root_inode_block;
    superblock.next_inode  = root_inode_block;
    strcpy(superblock.check, "SUPERBLOCK");
    update_superblock();
    set_superblock(sb_id);

    // Set up root data directory
    struct file listing[2];
    listing[0].block  = superblock.root_inode;
    listing[0].offset = 0;
    listing[0].name   = ".";

    listing[1].block  = superblock.root_inode;
    listing[1].offset = 0;
    listing[1].name   = "..";
    
    char data[2048];
    int len = pack_listing(listing, 2, data);
    write_block(root_inode_data, data, len);

    // Set up root inode
    int now = time(0);
    struct inode root_inode;
    root_inode.block    = root_inode_block;
    root_inode.offset   = 0;
    root_inode.size     = len;
    root_inode.nlinks   = 2;
    root_inode.mode     = G_IFDIR | 0666;
    root_inode.created  = now;
    root_inode.modified = now;
    root_inode.level1   = root_inode_lvl1;
    root_inode.level2   = 0;
    root_inode.level3   = 0;
    update_inode(&root_inode);

    // Write Level1
    struct block_pointers ptrs;
    ptrs.blocks[0] = root_inode_data;
    write_block(root_inode_lvl1, (char*) &ptrs, 4);
}

// Helper function definitions

struct inode get_inode(BID block, uint8_t offset) {
    char data[2048];
    read_block(block, data);

    struct inode* inodes = ((struct inode_block*) data)->inodes;
    return inodes[offset];
}

int update_superblock() {
    char* sb_data = (char*) &superblock;
    write_block(superblock.address, sb_data, sizeof(struct superblock));

    return 0;
}

int update_inode(struct inode* inode) {
    char data[BLOCK_SIZE];
    read_block(inode->block, data);

    char* bin_inode = (char*) inode;
    int offset = INODE_SIZE * inode->offset;
    for(int i=0; i<INODE_SIZE; i++) {
        data[offset+i] = bin_inode[i];
    }

    write_block(inode->block, data, 2048);

    return 0;
}

BID get_nth_block(long n, struct inode* inode) {
    // TODO: Level 2 & 3,
    // seize new block when block address found is 0
    if(n < 512) {
        // Level 1
        char data[2048];
        read_block(inode->level1, data);

        struct block_pointers* level1 = (struct block_pointers*) data;

        return level1->blocks[n];
    } else if( n-512 < 512*512 ) {
        // Level 2
        n -= 512;
    } else if( n-512-512*512 < 512*512*512 ) {
        // Level 3
        n -= (512 + 512*512);

    }
}

int pack_listing(struct file* listing, int nbFiles, char* buffer) {
    int c=0;

    for(int i=0; i<nbFiles; i++) {
        char* addr_bin = (char*) &listing[i].block;
        for(int j=0; j<4; j++)
            buffer[c++] = addr_bin[j];
        buffer[c++] = listing[i].offset;
        int len = strlen(listing[i].name);
        for(int j=0; j<len; j++)
            buffer[c++] = listing[i].name[j];
        buffer[c++] = 0x03;
    }

    return c;
}

void print_inode(struct inode *i) {
    printf("--- Inode\n");
    printf("Address: %d | Offset %d\n", i->block, i->offset);
    printf("Size: %d\n", i->size);
    printf("Mode: %u\n", i->mode);
    printf("nlinks: %d\n", i->nlinks);
    printf("Created: %d\n", i->created);
    printf("Modified: %d\n", i->modified);
    printf("Level1: %d\n", i->level1);
    printf("Level2: %d\n", i->level2);
    printf("Level3: %d\n", i->level3);
}

int get_filename(const char *path, char *filename) {
	char *pStrippedFilename = strrchr(path,'/');
	if (pStrippedFilename!=NULL) {
		++pStrippedFilename;
		if ((*pStrippedFilename) != '\0') {
			strcpy(filename, pStrippedFilename);
			return 1;
		}
	}
	return 0;
}

int get_parent(const char *path, char *parent) {
	strcpy(parent,path);
	int len = strlen(parent);
	int index;
	
	while (parent[len]!='/') {
		len--;
		if (len <0) {
			return 0;
		}
	}
	if (len==0) { 
		parent[0] = '/';
		parent[1] = 0;
	}
	else {
		parent[len] = '\0';
	}
	return 1;
}