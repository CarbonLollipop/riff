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
   
    printf("Bye!\n");

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

    struct stat sb;

    char* queue[256];

    if (stat(argv[1], &sb) == 0 && S_ISDIR(sb.st_mode)) {
        int numSongs = 0;

        struct dirent* file;

        DIR* directory = opendir(argv[1]);

        while ((file = readdir(directory)) != NULL && numSongs < 256) {
            const char* extension = strrchr(file->d_name, '.');
            if (extension != NULL && (strcmp(extension, ".mp3") == 0 || strcmp(extension, ".wav") == 0 || strcmp(extension, ".flac") == 0)) {
                queue[numSongs] = (char*) malloc(256 * sizeof(char));
                strncpy(queue[numSongs], file->d_name, 255);
                queue[numSongs][255] = '\0';

                numSongs++;
            }
        }

        closedir(directory);
    } else if (stat(argv[1], &sb) == 0 && S_ISREG(sb.st_mode)) {
        queue[0] = argv[1];
    }

    initscr();
    nodelay(stdscr, TRUE);
    noecho();
  
    printw("(P)ause/(P)lay\n");
    printw("(S)kip\n");
    printw("(Q)uit\n-------\n");

    for(int i = 0; i < sizeof queue / sizeof queue[0]; i++) {
        Mix_Music* music = Mix_LoadMUS(queue[i]);
        
        if (!music) {
            printf("%s", Mix_GetError());
            return 1;
        }

        Mix_PlayMusic(music, 0);

        printw("Now playing %s\n", queue[i]);

        while (Mix_PlayingMusic()) {
            SDL_Delay(1);
            
            char c = getch();

            if (c == 'p')
                Mix_PausedMusic() ? Mix_ResumeMusic() : Mix_PauseMusic();
            else if(c == 'q')
                quit();
            else if(c == 's')
                Mix_HaltMusic();
        }
    
        Mix_FreeMusic(music);
    }

    quit();
}

