#include <stdio.h>
#include <signal.h>
#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void update(int volume, const char* songName, int paused) {
    deleteln();
    move(getcury(stdscr), 0);
    printw("%d \"%s\" ", volume, songName);
    paused ? printw(" >") : printw("||");
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

    // FIXME make me dynamic!!!!!!!
        
        char queue[64][256];

        // FIXME ./riff ./sound.mp3 WORKS BUT ./riff sound.mp3 DOES NOT

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
        keypad(stdscr, TRUE);
        noecho();
        curs_set(0); 

        if(songs > 1)
        printw("  S                    Skip\n");
        printw("  Spacebar             Pause/Play\n");
        printw("  Q                    Quit\n");
        printw("  Up/Down Arrow        Adjust volume\n\n");

        int volume = 128;

        for(int i = 0; i < songs; i++) {
            Mix_Music* music = Mix_LoadMUS(queue[i]);
            
            if (!music) {
                printf("%s", Mix_GetError());
                return 1;
            }

            Mix_PlayMusic(music, 0);
            
            int paused = 0;

            const char* songName = strrchr(queue[i], '/')+1;
            
            update(volume, songName, paused);

            while (Mix_PlayingMusic()) {
                SDL_Delay(1);
                
                int c = getch();

                if (c == 'p' || c == ' ') {
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
                    update(volume, songName, paused);
                } else if(c == 'q') {
                    quit();
                } else if(c == 's' && songs > 1) {
                    Mix_HaltMusic();
                } else if(c == KEY_DOWN) {
                    if(volume >= 16) {
                        volume -= 16;
                        Mix_VolumeMusic(volume); 
                    }
                    update(volume, songName, paused);
                } else if(c == KEY_UP) {
                    if(volume < 128) {
                        volume += 16;
                        Mix_VolumeMusic(volume); 
                    }

                    update(volume, songName, paused);
                }
            }

            deleteln();
            move(getcury(stdscr), 0);
       
            Mix_FreeMusic(music);
        }

        quit();
}

