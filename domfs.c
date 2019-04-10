#include "domfs.h"

struct superblock superblock;

// Helper functions headers
struct inode get_inode(BID block, uint8_t offset);
BID get_nth_block(long n, struct inode* inode);
int update_inode(struct inode* inode);
int update_superblock();
long pack_listing(struct file* listing, int nbFiles, char* buffer);
int unpack_listing(struct file* listing, char* packed, long length, int nbFiles);
int seize_inode(struct inode* inode);
int get_filename(const char* path, char *filename);
int get_parent(const char* path, char* parent);
int dir_add_data(struct inode* inode, struct file* entry);
int add_to_listing(struct file* old, struct file* new, int nb);
long min(long a, long b) { return a < b ? a : b; }
long max(long a, long b) { return a > b ? a : b; }

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

// Creates a file
// Returns:
//  0 : Success
// -1 : Parent directory not found
// -2 : File already exists
// -3 : Permission error
// -4 : Path error
int fs_create(const char* path) {
    char filename[256];
    char parent_path[1024];
    get_filename(path, filename);
    get_parent(path, parent_path);

    struct inode inode, parent_inode;

    if(fs_getattr(parent_path, &parent_inode) == -1)
        return -1; // Parent directory not found
    if(!(parent_inode.mode & G_IFDIR))
        return -4; // Parent "directory" is not a directory
    if(!(parent_inode.mode & G_IWUSR))
        return -3; // No permission to write in parent directory
    if(fs_getattr(path, &inode) == 0)
        return -2; // File already exists

    // Creates file inode
    seize_inode(&inode);
    inode.size     = 0;
    inode.mode     = G_IFREG | 0666;
    inode.nlinks   = 1;
    inode.created  = time(0);
    inode.modified = time(0);
    inode.level1   = 0;
    inode.level2   = 0;
    inode.level3   = 0;
    printf("Create - update inode\n");
    update_inode(&inode);

    // Add to parent's directory data
    struct file entry;
    entry.block = inode.block;
    entry.offset = inode.offset;
    strcpy(entry.name, filename);
    printf("Create - add to dir data\n");
    dir_add_data(&parent_inode, &entry);

    // Update parent inode
    parent_inode.modified = time(0);
    parent_inode.size += strlen(filename) + 6;
    printf("Create - update parent\n");
    update_inode(&parent_inode);
    
    return 0;
}

// Creates a directory
// Returns:
//  0 : Success
// -1 : Parent directory not found
// -2 : File already exists
// -3 : Permission error
// -4 : Path error
int fs_mkdir(const char* path) {
    char dirname[256];
    char parent_path[1024];
    get_filename(path, dirname);
    get_parent(path, parent_path);

    struct inode inode, parent_inode;

    if(fs_getattr(parent_path, &parent_inode) == -1)
        return -1; // Parent directory not found
    if(!(parent_inode.mode & G_IFDIR))
        return -4; // Parent "directory" is not a directory
    if(!(parent_inode.mode & G_IWUSR))
        return -3; // No permission to write in parent directory
    if(fs_getattr(path, &inode) == 0)
        return -2; // File already exists

    // Setup inode
    seize_inode(&inode);
    inode.size     = 15;
    inode.mode     = G_IFDIR | 0666;
    inode.nlinks   = 2;
    inode.created  = time(0);
    inode.modified = time(0);
    inode.level1   = 0;
    inode.level2   = 0;
    inode.level3   = 0;
    update_inode(&inode);

    // Setup directory data
    struct file listing[2];
    listing[0].block  = parent_inode.block;
    listing[0].offset = parent_inode.offset;
    strcpy(listing[0].name, "..");
    listing[1].block  = inode.block;
    listing[1].offset = inode.offset;
    strcpy(listing[1].name, ".");

    // Write in data block
    char data[15];
    pack_listing(listing, 2, data);
    write_block(get_nth_block(0, &inode), data, 15);

    // Update parent directory's listing
    struct file entry;
    entry.block  = inode.block;
    entry.offset = inode.offset;
    strcpy(entry.name, dirname);
    dir_add_data(&parent_inode, &entry);

    // Update parent inode
    parent_inode.modified = time(0);
    parent_inode.nlinks += 1;
    parent_inode.size += strlen(dirname) + 6;
    update_inode(&parent_inode);
}

// Writes `length` bytes from buffer in file starting from specified offset
// Returns:
// ≥0 : Number of bytes written
// -1 : File not found
// -2 : Permission error
// -3 : File isn't a regular file
// -4 : Offset too big
int fs_write(const char* path, char* buffer, int offset, int length) {
    struct inode inode;

    if(fs_getattr(path, &inode) == -1)
        return -1;
    if(!(inode.mode & G_IWUSR))
        return -2;
    if(!(inode.mode & G_IFREG))
        return -3;
    printf("Offset: %d, size: %d\n", offset, inode.size);
    if(offset > inode.size)
        return -4;

    // Update inode
    inode.modified = time(0);
    inode.size = max(offset+length, inode.size);
    update_inode(&inode);

    // Write in data blocks

    int p = 0;
    int nbBytesLeft = length;
    int block = offset / 2048;
    int pos_in_block = offset % 2048;
    char data[2048];

    while(nbBytesLeft > 0) {
        BID addr = get_nth_block(block++, &inode);
        read_block(addr, data);

        int w=0;
        for(int i=pos_in_block; i < min(2048, pos_in_block + nbBytesLeft); i++) {
            data[i] = buffer[p++];
            w++;
        }

        write_block(addr, data, 2048);

        nbBytesLeft -= w;
        pos_in_block = 0;
    }

    return p;
}

// Formats the "disk" i.e. posts and pins a new superblock and initializes the root directory
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
    strcpy(listing[0].name, ".");

    listing[1].block  = superblock.root_inode;
    listing[1].offset = 0;
    strcpy(listing[1].name, "..");
    
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

int seize_inode(struct inode* inode) {
    BID block = superblock.next_inode;
    char data[2048];
    read_block(block, data);

    struct inode* inodes = ((struct inode_block*) data)->inodes;

    for(int i=0; i<64; i++) {
        if(inodes[i].block == 0) {
            inode->block = block;
            inode->offset = i;

            return 0;
        }
    }

    // No free inode found in block, seize new one
    superblock.next_inode = seize_block();
    update_superblock();
    inode->block = superblock.next_inode;
    inode->offset = 0;

    return 0;
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

// Returns the address of the nth data block
// belonging to a given inode. Is in charge of following links
// between blocks from 1, 2 or 3 levels of references.
// Seizes new blocks if they're not available.
BID get_nth_block(long n, struct inode* inode) {
    char data[2048];
    
    if(n < 512) {
        // Level 1
        if(inode->level1 == 0) {
            inode->level1 = seize_block();
            update_inode(inode);
        }

        read_block(inode->level1, data);

        struct block_pointers* level1 = (struct block_pointers*) data;
        if(level1->blocks[n] == 0) {
            level1->blocks[n] = seize_block();
            write_block(inode->level1, (char*) level1, 2048);
        }

        return level1->blocks[n];
    } else if( n-512 < 512*512 ) {
        // Level 2
        if(inode->level2 == 0) {
            inode->level2 = seize_block();
            update_inode(inode);
        }
        n -= 512;
        int l2 = n / 512;
        int l1 = n % 512;

        read_block(inode->level2, data);
        struct block_pointers* level2 = (struct block_pointers*) data;

        if(level2->blocks[l2] == 0) {
            level2->blocks[l2] = seize_block();
            write_block(inode->level2, (char*) level2, 2048);
        }

        read_block(level2->blocks[l2], data);
        struct block_pointers* level1 = (struct block_pointers*) data;

        if(level1->blocks[l1] == 0) {
            level1->blocks[l1] = seize_block();
            write_block(level2->blocks[l2], (char*) level2, 2048);
        }

        return level1->blocks[l1];
        
    } else if( n-512-512*512 < 512*512*512 ) {
        // Level 3
        if(inode->level3 == 0) {
            inode->level3 = seize_block();
            update_inode(inode);
        }
        n -= (512 + 512*512);

        int l3 = n / (512*512);
        n = n % (512*512);
        int l2 = n / 512;
        int l1 = n % 512;

        read_block(inode->level3, data);
        struct block_pointers* level3 = (struct block_pointers*) data;

        if(level3->blocks[l3] == 0) {
            level3->blocks[l3] = seize_block();
            write_block(inode->level3, (char*) level3, 2048);
        }

        read_block(level3->blocks[l3], data);
        struct block_pointers* level2 = (struct block_pointers*) data;

        if(level2->blocks[l2] == 0){
            level2->blocks[l2] = seize_block();
            write_block(level3->blocks[l3], (char*) level2, 2048);
        }

        read_block(level2->blocks[l2], data);
        struct block_pointers* level1 = (struct block_pointers*) data;

        if(level1->blocks[l1] == 0){
            level2->blocks[l1] = seize_block();
            write_block(level2->blocks[l1], (char*) level2, 2048);
        }

        return level1->blocks[l1];
    }
}

// Pack a list of files into a binary string
// buffer must have enough space
long pack_listing(struct file* listing, int nbFiles, char* buffer) {
    long c=0;

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

// Get the list of files from a binary string
int unpack_listing(struct file* listing, char* packed, long length, int nbFiles) {
    char name[256][nbFiles];
    int name_pos = -2;
    char bin_block[5];
    int bin_block_pos = 0;
    int f = 0;
    char byte;

    for(int i=0; i<length; i++) {
        byte = packed[i];
        if(name_pos == -2) {
            // Block
            bin_block[bin_block_pos++] = byte;
            if(bin_block_pos == 4) {
                bin_block_pos = 0;
                name_pos = -1;
                listing[f].block = *((BID*) bin_block);
            }
        } else if (name_pos == -1) {
            // Offset
            listing[f].offset = byte;
            name_pos = 0;
        } else {
            if(byte == 0x03) {
                // End of file name
                name[f][name_pos] = 0x00;
                strcpy(listing[f].name, name[f]);
                
                name_pos = -2;
                f++;
            } else {
                name[f][name_pos++] = byte;
            }
        }
    }
}

// Adds an entry in a directory data listing
// Keeps everything sorted
int dir_add_data(struct inode* inode, struct file* entry) {
    long size = inode->size;
    char old_data[size];
    int nbFiles = 0;

    char buffer[2048];
    int block = 0;
    long nbBytesRead = 0;
    int pos = 0;

    // Read entire dir data and get the number of files at the same time
    while(nbBytesRead < size) {
        read_block(get_nth_block(block++, inode), buffer);
        int i;
        for(i=0; i < min(2048, size - nbBytesRead); i++) {
            old_data[nbBytesRead+i] = buffer[i];

            if(pos > 5 && buffer[i] == 0x03) {
                nbFiles++;
                pos = 0;
            }
            pos++;
        }
        nbBytesRead += i;
    }

    // Parse entire data fetched in old_data into a list of files
    struct file old_listing[nbFiles];
    unpack_listing(old_listing, old_data, size, nbFiles);

    // Add new entry where it belongs
    struct file new_listing[nbFiles+1];
    strcpy(new_listing[0].name, entry->name);
    new_listing[0].block = entry->block;
    new_listing[0].offset = entry->offset;

    add_to_listing(old_listing, new_listing, nbFiles+1);

    // Pack new list and write in blocks
    char new_data[size + strlen(entry->name) + 6];
    int newSize = pack_listing(new_listing, nbFiles+1, new_data);

    int nbBytesWritten = 0;
    block = 0;
    int c = 0;
    while(nbBytesWritten < newSize) {
        char data[2048];
        int i;
        int blocksize = min(2048, newSize-nbBytesWritten);

        for(i=0; i<blocksize; i++)
            data[i] = new_data[c++];

        write_block(get_nth_block(block++, inode), data, blocksize);
        nbBytesWritten += i;
    }
}

int add_to_listing(struct file* old, struct file* new, int nb) {
    int added = 0;
    struct file entry = *new;
    for(int i=0; i<nb; i++) {
        if(!added) {
            if(i==nb-1 || strcmp(old[i].name, entry.name) > 0) {
                strcpy(new[i].name, entry.name);
                new[i].block = entry.block;
                new[i].offset = entry.offset;
                added = 1;
            }
        }
        if(i < nb-1) {
            strcpy(new[i+added].name, old[i].name);
            new[i+added].block = old[i].block;
            new[i+added].offset = old[i].offset;
        }
    }
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