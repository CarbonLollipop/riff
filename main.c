#include <stdio.h>
#include <signal.h>
#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void quit() {
    endwin();
   
    Mix_CloseAudio();
    SDL_Quit();
    
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

    // FIXME make me dynamic!!!!!!!
    
    char queue[64][256];

        uint songs = 0;
    for(int i = 1; i < argc; i++) {

        struct stat sb;

        if (stat(argv[i], &sb) == 0 && S_ISDIR(sb.st_mode)) {
            struct dirent* file;

            DIR* directory = opendir(argv[i]);

            while ((file = readdir(directory)) != NULL ) {
                const char* extension = strrchr(file->d_name, '.');

                if (extension != NULL && (strcmp(extension, ".mp3") == 0 || strcmp(extension, ".wav") == 0 || strcmp(extension, ".flac") == 0)) {
                    char resolvedPath[256];
                    realpath(argv[i], resolvedPath);
                    
                    strcat(resolvedPath, "/");
                    char* fullname = strcat(resolvedPath, file->d_name);
                    strncpy(queue[songs], fullname, 255);
                    songs++;
                }
            }

            closedir(directory);
        } else if (stat(argv[i], &sb) == 0 && S_ISREG(sb.st_mode)) {
            strncpy(queue[songs], argv[i], 255);
            songs++;
        }
    }

    initscr();
    nodelay(stdscr, TRUE);
    noecho();
  
    printw("P     Pause/Play\n");
    printw("S     Skip\n");
    printw("Q     Quit\n");
    printw("+ / - Adjust volume\n\n");

    for(int i = 0; i < songs; i++) {
        Mix_Music* music = Mix_LoadMUS(queue[i]);
        
        if (!music) {
            printf("%s", Mix_GetError());
            return 1;
        }

        Mix_PlayMusic(music, 0);

        const char* songName = strrchr(queue[i], '/')+1;
        printw("|| %s", songName);

        while (Mix_PlayingMusic()) {
            SDL_Delay(1);
            
            char c = getch();

            if (c == 'p') {
                deleteln();
                move(getcury(stdscr), 0);
                if(Mix_PausedMusic()) {
                    Mix_ResumeMusic();
                    printw("||");
                } else {
                    Mix_PauseMusic();
                    printw("> ");
                }
                   
                printw(" %s", songName);
            } else if(c == 'q') {
                quit();
            } else if(c == 's') {
                Mix_HaltMusic();
            } else if(c == '-') {
                if(Mix_VolumeMusic(-1) >= 16)
                Mix_VolumeMusic(Mix_VolumeMusic(-1) - 16);
            }  else if(c == '+') {
                if(Mix_VolumeMusic(-1) <= 128)
                Mix_VolumeMusic(Mix_VolumeMusic(-1) + 16);
            }
        }

        deleteln();
        move(getcury(stdscr), 0);
   
        Mix_FreeMusic(music);
    }

    quit();
}

