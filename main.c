#include <stdio.h>
#include <signal.h>
#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

int compare(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void sort(char* queue[], int n) {
    qsort(queue, n, sizeof(char*), compare);
}

void update(int volume, const char* songName, int paused, Mix_Music *music) {
    deleteln();
    move(getcury(stdscr), 0);
    char volumeString[8];

    for(int i = 0; i < 8; i++) {
        volumeString[i] = (i < volume / 16) ? ':' : '.';
    }

    printw("%s [%s] ", songName, volumeString);
    paused ? printw("Paused") : printw("Playing");
}

void quit() {
    endwin();

    Mix_CloseAudio();
    SDL_Quit();

    curs_set(1); 
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <song/directory of songs>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to initialize SDL audio system: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Failed to open the audio device: %s\n", Mix_GetError());
        return 1;
    }

    signal(SIGINT, quit);

    char* queue[256];

    unsigned int songs = 0;
    
    for(int i = 1; i < argc; i++) {

        struct stat sb;

        if (stat(argv[i], &sb) == 0 && S_ISDIR(sb.st_mode)) {
            struct dirent* file;

            DIR* directory = opendir(argv[i]);

            while ((file = readdir(directory)) != NULL ) {
                const char* extension = strrchr(file->d_name, '.');

                if (extension != NULL && (strcmp(extension, ".mp3") == 0 || strcmp(extension, ".wav") == 0 || strcmp(extension, ".flac") == 0)) {
                    queue[songs] = (char*)malloc(128 * sizeof(char));
                    char resolvedPath[256];
                    realpath(argv[i], resolvedPath);
                    
                    strcat(resolvedPath, "/");
                    char* fullname = strcat(resolvedPath, file->d_name);
                    strcpy(queue[songs], fullname);
                    songs++;
                }
            }

            closedir(directory);
        } else if (stat(argv[i], &sb) == 0 && S_ISREG(sb.st_mode)) {
            queue[songs] = (char*)malloc(128 * sizeof(char));
            char resolvedPath[256];
            realpath(argv[i], resolvedPath);
            strcpy(queue[songs], resolvedPath);
            songs++;
        }

    }

    sort(queue, songs);

    initscr();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0); 

    if(songs > 1) {
        printw("N                Next\n");
        printw("P                Previous\n");
    }
    printw("Spacebar         Pause/Play\n");
    printw("Q                Quit\n");
    printw("Up/Down Arrow    Adjust volume\n\n");

    int volume = 128;
    int paused = 0;
   
    for(int i = 0; i < songs; i++) {
        Mix_Music* music = Mix_LoadMUS(queue[i]);
        
        if (!music) {
            printf("%s", Mix_GetError());
            return 1;
        }

        Mix_PlayMusic(music, 0);
        
        const char* songName = strrchr(queue[i], '/') + 1;
        
        update(volume, songName, paused, music);

        while (Mix_PlayingMusic()) {
            SDL_Delay(20);
            
            int c = getch();

            if (c == ' ') {
                if(paused) {
                    Mix_VolumeMusic(volume);
                    SDL_Delay(3);
                    Mix_ResumeMusic();
                    paused = 0;
                } else {
                    Mix_VolumeMusic(0);
                    SDL_Delay(3);
                    Mix_PauseMusic();
                    paused = 1;
                }
                update(volume, songName, paused, music);
            } else if(c == 'q') {
                quit();
            } else if(c == 'n' && songs > 1) {
                Mix_HaltMusic();
            } else if(c == 'p' && songs > 1 && i > 0) {
                Mix_HaltMusic();
                i -= 2;
            } else if(c == KEY_DOWN) {
                if(volume >= 16) {
                    volume -= 16;
                    Mix_VolumeMusic(volume); 
                }
                update(volume, songName, paused, music);
            } else if(c == KEY_UP) {
                if(volume < 128) {
                    volume += 16;
                    Mix_VolumeMusic(volume); 
                }

                update(volume, songName, paused, music);
            }
        }

        Mix_FreeMusic(music);
    }

    quit();
}

