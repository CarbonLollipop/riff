CC=gcc
CFLAGS=-Wall -Wextra

riff: main.o
	gcc -o riff main.o -lSDL2_mixer -Wl,-Bstatic -lSDL2 -Wl,-Bdynamic -lm -lasound -lncurses -lpulse -lX11 -lXext -lXcursor -lXfixes -lXinerama -lXi -lXrandr -lXss -lXxf86vm -ldrm -lgbm -lwayland-server -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon -ldecor-0 -Wl,-rpath=/usr/local/lib -L/usr/local/lib

main.o: main.c
	gcc -c -o main.o main.c

clean:
	rm -f riff main.o

