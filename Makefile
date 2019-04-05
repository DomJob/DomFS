
main: main.c domfs disk telegram dirs
	gcc main.c bin/telegram.o bin/disk.o bin/domfs.o  -l:libtdjson.so -g -o domfs

domfs: domfs.c
	gcc -c -g domfs.c -o bin/domfs.o

disk: disk.c
	gcc -c -g disk.c -o bin/disk.o

telegram: telegram.c
	gcc -c -g telegram.c -o bin/telegram.o

dirs:
	mkdir -p bin && mkdir -p data

clean: 
	rm -f bin/*
	rm -f domfs
