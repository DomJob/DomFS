**Project on standby until I figure out a better structure to get around harsh rate limits**

**This is still a work in progress.**

# DomFS

`domfs` is a small C project that simulates a file system and stores the data in a telegram supergroup in hexadecimal form.

It uses FUSE (File system in user space) to mount domfs to any given mountpoint and navigate directories as you would on any hard drive or usb flash drive.

Yes, I named the project after myself because I couldn't think of a better name and something similar has already been done so [tgfs](https://github.com/Firemoon777/tgfs) is already taken

## Build

Note: Because of FUSE (and many other development-related reasons), DomFS is only compatible with *nix systems. This was developped and tested on a machine running Manjaro.

**Dependencies:**

* libfuse-dev
* libtdjson\.so
   * You can build it from source from [here](https://github.com/tdlib/td/) or find a `libtdjson.so` binary elsewhere online.
   * Put `libtdjson.so` in `/usr/lib/`

Simply run `make` and everything should build fine.

## Setup

1. Create a dedicated supergroup on telegram, make your client bot admin with full privileges (if you're not running it on your main account)

2. Rename `config.ini.example` to `config.ini` and put in your api key and hash. You can get them from [my.telegram.org](https://my.telegram.org). You can leave `chat` and `supergroup` as `0` if you don't know the exact IDs.

3. Run `./domfs mount` (you have to call the binary from its main folder) and follow prompt instructions to login and setup the chat. Then as long as the process is running, you can navigate files mounted to the specified mount point!

## Warning

There is a good chance that the phone number you use gets banned the second you enter the verification code in the prompt. This is a [known issue](https://github.com/tdlib/td/issues/312) that I encountered myself. If that happens, simply email recover@telegram.org and give them your phone number and explain you are simply using the official TDLib API and got banned by mistake. They are quick to reply (3-12 hours) and will unban your number. Please do not mention this most likely ToS breaking project in your email. Oh, that reminds me: Don't use DomFS for anything serious, and always backup your important data!

## More information

The maximum message length on Telegram is 4096 characters, so this represents 2048 bytes of binary data when encoded in hexadecimal. (I know I could encode way more with unicode characters but this is simpler)

So a message contains up to 2048 bytes, which is our file system's block. The data is first accessed through the superblock which is the pinned message in the chat. This block contains information like the root directory's inode and from there everything can be decoded given references to other blocks that `domfs` reads and edits. The file system's structure is highly inspired on the old school [Unix File System](https://en.wikipedia.org/wiki/Unix_File_System)

You can find more information about the structure [here](structure.md) but that file was more of a reference for myself during development.