
main: main.c domfs disk telegram
	gcc main.c telegram.o disk.o domfs.o -lpthread -l:libtdjson.so -lfuse -g -o domfs -D_FILE_OFFSET_BITS=64 -Wall

domfs: domfs.c
	gcc -c -g domfs.c -o domfs.o

disk: disk.c
	gcc -c -g disk.c -o disk.o

telegram: telegram.c
	gcc -c -g telegram.c -o telegram.o

clean:
	rm -f *.o
	rm -f domfs
