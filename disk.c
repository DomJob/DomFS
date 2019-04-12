#include "disk.h"

char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
char hex2byte(char first, char second);

int mounted;
pthread_t write_thread;
void *write_loop();

void disk_initialize() {
    mkdir("./cache", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    pthread_create(&write_thread, NULL, write_loop, NULL);
    mounted = 1;
}

void disk_release() {
    mounted = 0;
    pthread_join(write_thread, NULL);
}

BID seize_block() {
    char* msg = "0";

    while(1) {
        BID id = tg_send_message(msg);
        if(id > 0)
            return id;
        sleep(1);
    }
}

void read_block(BID id, char* data) {
    char hexdata[2*BLOCK_SIZE];

    for(int i=0; i<BLOCK_SIZE; i++) {
        data[i]        = '\0';
        hexdata[2*i]   = '0';
        hexdata[2*i+1] = '0';
    }

    char filename[21];
    sprintf(filename, "./cache/%d.blk", id);
    FILE* fp = fopen(filename, "r");
    
    if(fp != NULL) {
        // Read from cache if file exists
        for(int i=0; i<4096; i++) {
            char nibble = fgetc(fp);
            if(nibble == EOF)
                break;
            hexdata[i] = nibble;
        }
        fclose(fp);
    } else {
        // Read from Telegram otherwise
        tg_read_message(id, hexdata);
    }

    // Convert hexdata to ascii bytes
    for(int i=0; i<BLOCK_SIZE; i++) {
        char h1 = hexdata[2*i];
        char h2 = hexdata[2*i+1];

        data[i] = hex2byte(h1, h2);
    }
}

void write_block(BID id, char* data, int length) {
    char hexdata[2*length];

    for(int i=0; i<length; i++) {
        unsigned char byte = data[i];
        hexdata[2*i]   = hex[byte >> 4];
        hexdata[2*i+1] = hex[byte - (byte >> 4 << 4)];
    }

    // Trim right-most zeros 
    int hexlen;
    for(hexlen = 2*length-1; hexlen>1; hexlen--) {
        if(hexdata[hexlen] != '0') 
            break;
    }
    hexdata[hexlen+1] = '\0';

    // Write to a (possibly new) cache file
    // write_thread is in charge of actually committing block edits
    char filename[21];
    sprintf(filename, "./cache/%d.blk", id);
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "%s", hexdata);
    fclose(fp);
}

BID get_superblock() {
    return tg_get_pinned_message();
}

void set_superblock(BID id) {
    tg_pin_message(id);
}

char hex2byte(char first, char second) {
    if(97 <= first)
        first -= 39;
    if(97 <= second)
        second -= 39;
    if(65 <= first)
        first -= 7;
    if(65 <= second)
        second -= 7;

    first -= 48;
    second -= 48;
    char byte = (first << 4) + second;
    return byte;
}

// This function is to be run in a separate thread.
// It is in charge of writing cached blocks to Telegram
// Pick a .blk file -> commit changes -> delete .blk file
// It basically has the job of avoiding Telegram's rate limit
// by minimizing how many edit requests are actually made
void *write_loop() {
    while(1) {
        DIR* d = opendir("./cache");
        struct dirent *file;
        file = readdir(d);
        while(file != NULL && (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)) {
            file = readdir(d);
        }

        if(file == NULL) {
            if(!mounted)
                break;
            sleep(1);
            continue;
        }

        char* name = file->d_name;

        BID block_id = 0;
        for(int i=0; i<strlen(name); i++) {
            if(name[i] == '.')
                break;
            block_id *= 10;
            block_id += (name[i] - 48);
        }
        
        char path[25];
        sprintf(path, "./cache/%s", name);
        
        FILE* f = fopen(path, "r");
        if(f == NULL) {
            printf("%s how come?\n", path);
            sleep(1);
            continue;
        }
        char data[4096];
        int i;
        for(i=0; i<4096; i++) {
            char c = fgetc(f);
            if(c == EOF) {
                data[i] = 0;
                break;
            }
            data[i] = c;
        }

        int success = -1;
        while(success == -1) {
            success = tg_edit_message(block_id, data);
            sleep(1);
        }
        
        unlink(path);
        
        sleep(1);
    }
}