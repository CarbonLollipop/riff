CC=gcc
CFLAGS=-Wall -Wextra

riff: main.o
	gcc -o riff main.o -lmpg123 -lSDL2_mixer -lSDL2 -Wl,-rpath=/usr/local/lib -L/usr/local/lib

main.o: main.c
	gcc -c -o main.o main.c

clean:
	rm -f riff

