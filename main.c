#include <stdio.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <ncurses.h>

void sigint_handler(int sig) {
    // TODO Hide that pesky little ^C

    printf("\nBye!\n");
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <song>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, sigint_handler);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to initialize SDL audio system: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Failed to open the audio device: %s\n", Mix_GetError());
        return 1;
    }
    
    Mix_Music* music = Mix_LoadMUS(argv[1]);
    
    if (!music) {
        printf("%s", Mix_GetError());
        return 1;
    }
    
    Mix_PlayMusic(music, 1);

    initscr();
    noecho();   

    printw("Playing %s\n", argv[1]);
    printw("P to pause/resume\n");

    while (Mix_PlayingMusic()) {
        if (getch() == 'p') {
            if (Mix_PausedMusic()) {
                Mix_ResumeMusic();
            } else {
                Mix_PauseMusic();
            }
        }
        SDL_Delay(100);
    }
   
    endwin();
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
    
    return 0;
}

