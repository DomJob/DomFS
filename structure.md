    address of a block  : 4 bytes
    address of an inode : block + offset = 4 bytes + 6 bits = 38 bits

    32 byte inodes in 2048 byte blocks = 64 inodes per block = 6 bit offset.

    (pinned) superblock
        "SUPERBLOCK"        10 bytes
        block nb            4 bytes
        secret              32 bytes
        root inode          5 bytes
        next free inode     5 bytes

    inode:              32 bytes
        block           4 bytes

        --- 64 bit fields (8 bytes)
        offset          6 bits
        size (on disk)  38 bits
        mode            6 bit flags
                            - is directory
                            - is regular file
                            - is link
                            - RWX user only
        number files    14 bits
        ---

        created         4 bytes
        modified        4 bytes

        data blocks     12 bytes
            Level 1:    4 bytes
            Level 2:    4 bytes
            Level 3:    4 bytes

            * max file size: 275.4 GB *

    dir data:
        -List of: (possibly fragmented in data blocks)
            inode address       : 4 bytes + 1 byte
            name                : arbitrary many bytes
                                  but limited to 256 by the implementation
            File name separator : 1 byte (0x03)


    data blocks: 2048 bytes of data