#include <stdio.h>
#include <signal.h>
#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <sndfile.h>
#include <mpg123.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void formatTime(int seconds, int *minutes, int *seconds_remaining) {
  *minutes = seconds / 60;
  *seconds_remaining = seconds % 60;
}

void shuffleQueue(char* queue[], int n) {
    srand(time(NULL));

    for(int i = 0; i < n; i++) {
        int j = rand() % n;
        char* temp = queue[i];
        queue[i] = queue[j];
        queue[j] = temp;
    }
}

int compare(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void sortQueue(char* queue[], int n) {
    qsort(queue, n, sizeof(char*), compare);
}

int showingHelp = 0;

void update(int volume, const char* songName, int paused, Mix_Music *music, double duration, int elapsed) {
   clear();
   
   if(showingHelp) {
        printw("H                Toggle Help\n");
        printw("N                Next\n");
        printw("P                Previous\n");
        printw("Spacebar         Pause/Play\n");
        printw("Q                Quit\n");
        printw("Up/Down Arrow    Adjust volume\n\n");
   } else {
        printw("Press H for help\n\n");
   }
  
   char volumeString[8];

    // code like this makes me feel so smart
    for(int i = 0; i < 8; i++)
        volumeString[i] = (i < volume / 16) ? ':' : '.';

    int elapsedMinutes, elapsedSeconds, durationMinutes, durationSeconds;
    formatTime(elapsed, &elapsedMinutes, &elapsedSeconds);
    formatTime((int)duration, &durationMinutes, &durationSeconds);
    
    // this is ugly!!

    printw("%02i:%02i / %02i:%02i %s [%s] ", elapsedMinutes, elapsedSeconds, durationMinutes, durationSeconds, songName, volumeString);
    paused ? printw("Paused") : printw("Playing");
}

double getDuration(const char* filePath) {
    const char* extension = strrchr(filePath, '.');
    
    if (!strcmp(extension, ".mp3")) {
        mpg123_init();
        mpg123_handle* handle = mpg123_new(NULL, NULL);
        mpg123_open(handle, filePath);
        long sampleRate = 0;
        mpg123_getformat(handle, &sampleRate, NULL, NULL);
        double duration = (double)mpg123_length(handle) / sampleRate;
        mpg123_delete(handle);
        mpg123_exit();
        return duration;
    } else {
        SF_INFO sfinfo;
        SNDFILE* sndfile = sf_open(filePath, SFM_READ, &sfinfo);
        double duration = (double)sfinfo.frames / sfinfo.samplerate;
        sf_close(sndfile);
        return duration;
    }
}

void quit() {
    endwin();

    Mix_CloseAudio();
    Mix_Quit();
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

    (argv[1][0] == '-' && argv[1][1] == 's') ? shuffleQueue(queue, songs) : sortQueue(queue, songs);

    initscr();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0); 

    int volume = 128;
    int paused = 0;
   
    for(int i = 0; i < songs; i++) {
        Mix_Music* music = Mix_LoadMUS(queue[i]);
        
        if (!music) {
            printw("%s", Mix_GetError());
            return 1;
        }

        double duration = getDuration(queue[i]);

        Mix_PlayMusic(music, 0);
        
        const char* songName = strrchr(queue[i], '/') + 1;
        
        time_t startTime = time(NULL);
        int counter = 0;
        int elapsedTime = 0;
        
        update(volume, songName, paused, music, duration, elapsedTime);

        while (Mix_PlayingMusic()) {
            SDL_Delay(20);
            
            int c = getch();
            
            time_t currentTime = time(NULL);
            int elapsedTime = (int)(currentTime - startTime);
           
            if (elapsedTime >= counter + 1) {
                counter++;
                update(volume, songName, paused, music, duration, elapsedTime);
            }

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
                update(volume, songName, paused, music, duration, elapsedTime);
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
                update(volume, songName, paused, music, duration, elapsedTime);
            } else if(c == KEY_UP) {
                if(volume < 128) {
                    volume += 16;
                    Mix_VolumeMusic(volume); 
                }
                update(volume, songName, paused, music, duration, elapsedTime);
            } else if(c == 'h') {
                showingHelp = (showingHelp == 1) ? 0 : 1;
                update(volume, songName, paused, music, duration, elapsedTime);
            }
        }

        Mix_FreeMusic(music);
    }

    nodelay(stdscr, FALSE);

    clear();
    printw("You've reached the end of the queue. Press any key to exit.");
    getch();

    quit();
}

