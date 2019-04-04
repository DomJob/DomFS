
main: main.c domfs disk
	gcc main.c bin/domfs.o bin/disk.o -g -o domfs

domfs: domfs.c
	gcc -c -g domfs.c -o bin/domfs.o

disk: disk.c
	gcc -c -g disk.c -o bin/disk.o

clean: 
	rm -f bin/*
	rm -f domfs
