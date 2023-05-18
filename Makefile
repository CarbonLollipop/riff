CC=gcc
CFLAGS=-Wall -Wextra

riff: main.o
	gcc -o riff main.o -lSDL2_mixer -lSDL2 -lncurses -lsndfile -lmpg123 -Wl,-rpath=/usr/local/lib -L/usr/local/lib

main.o: main.c
	gcc -c -o main.o main.c

clean:
	rm -f riff main.o

