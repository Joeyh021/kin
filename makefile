CC = gcc
CFLAGS = -Wall -Wextra -g
BUILDFLAGS = -lncurses -std=gnu11

main: src/*
	$(CC) src/*.c -o kin $(BUILDFLAGS) 

debug: bin/filehandler.o bin/gui.o src/main.c bin/common.o
	$(CC) src/main.c bin/gui.o bin/filehandler.o bin/common.o -o kin $(CFLAGS)  $(BUILDFLAGS) 

bin/gui.o: src/gui.c src/gui.h 
	$(CC) -c src/gui.c -o bin/gui.o $(CFLAGS) $(BUILDFLAGS) 

bin/filehandler.o: src/filehandler.c src/filehandler.h 
	$(CC) -c src/filehandler.c -o bin/filehandler.o $(CFLAGS) $(BUILDFLAGS) 

bin/common.o: src/common.c src/common.h
	$(CC) -c src/common.c -o bin/common.o $(CFLAGS) $(BUILDFLAGS) 

clean:
	rm -rf bin && rm kin