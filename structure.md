    address of a block  : 4 bytes
    address of an inode : 5 bytes : block + offset = 4 bytes + 6 bits (2 bits wasted)

    32 byte inodes in 2048 byte blocks = 64 inodes per block = 6 bits.

    (pinned) superblock
        "SUPERBLOCK"        10 bytes
        block nb            4 bytes
        secret              32 bytes
        root inode          5 bytes
        next free inode     5 bytes

    inode:              32 bytes
        number          5 bytes
        size (on disk)  5 bytes
        time created    4 bytes
        last modified   4 bytes
        number links    1 bytes 
        mode            1 byte (8 bit flags)
                            - Directory
                            - Regular file
                            - RWX
                            -  3 more spots
        data blocks     12 bytes
            Level 1:    4 bytes
            Level 2:    4 bytes 
            Level 3:    4 bytes

            * max file size: 275.4 GB *

    dir data:
        List of: (fragmented in data blocks)
            inode address    : 4 bytes + 1 byte
            nb bytes name    : 1 byte
            name             : 1 to 256 bytes

    data blocks: 2048 bytes of data