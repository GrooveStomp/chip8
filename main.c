/******************************************************************************
  File: main.c
  Created: 2019-06-04
  Updated: 2019-07-27
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#include "input.h"
#include "graphics.h"
#include "opcode.h"
#include "sound.h"
#include "system.h"
#include "timer.c"
#include "ui.h"

#include "threadsync.c"
struct thread_args {
        struct system *sys;
        struct opcode *opcode;
        int isDebugEnabled;
        struct thread_sync *threadSync;
};

#include "gfxinputthread.c"
#include "soundthread.c"
#include "timerthread.c"

static struct system *sys;
static struct opcode *opcode;
static struct thread_sync *threadSync;

static pthread_t timerThread;
static pthread_t soundThread;
static pthread_t gfxInputThread;

void Shutdown(int status) {
        void *threadStatus;
        ThreadSyncSignalShutdown(threadSync);
        pthread_join(timerThread, &threadStatus);
        pthread_join(soundThread, &threadStatus);
        pthread_join(gfxInputThread, &threadStatus);

        if (NULL != opcode)
                OpcodeDeinit(opcode);

        if (NULL != sys)
                SystemDeinit(sys);

        exit(status);
}


void Usage() {
        printf("chip-8 [-d] PROGRAM\n");
        printf("\t-d: interactive debug mode\n");
}

// Returns whether debug is enabled.
int ArgParse(int argc, char **argv) {
        int debugEnabled = 0;

        if (argc < 2 || argc > 3) {
                Usage();
                exit(1);
        }

        if (argc == 3 && strcmp(argv[1], "-d") == 0) {
                argv[1] = argv[2];
                debugEnabled = 1;
        } else if (argc == 3 && strcmp(argv[2], "-d") == 0) {
                debugEnabled = 1;
        } else if (argc == 3) {
                Usage();
                exit(1);
        }

        return debugEnabled;
}

int main(int argc, char **argv) {
        int debugEnabled = ArgParse(argc, argv);

        size_t fsize = 0;
        unsigned char *mem;
        {
                FILE *f = fopen(argv[1], "r");
                if (f == NULL) {
                        perror("Couldn't open file");
                        exit(1);
                }

                int c;
                while ((c = fgetc(f)) != EOF) {
                        fsize++;
                }

                if (ferror(f)) {
                        perror("Stopped reading file early");
                }

                clearerr(f);
                fseek(f, 0, SEEK_SET);

                int i = 0;
                mem = (unsigned char *)malloc(fsize);
                while ((c = fgetc(f)) != EOF) {
                        mem[i] = (char)c;
                        i++;
                }

                fclose(f);
        }

        threadSync = ThreadSyncInit();
        if (NULL == threadSync) {
                fprintf(stderr, "Couldn't initialize threadsync");
                exit(1);
        }

        sys = SystemInit(debugEnabled);
        if (!SystemLoadProgram(sys, mem, fsize)) {
                fprintf(stderr, "Couldn't load program into Chip-8");
                Shutdown(1);
        }

        opcode = OpcodeInit();
        if (NULL == opcode) {
                fprintf(stderr, "Couldn't initialize opcode");
                Shutdown(1);
        }

        int err;
        struct thread_args threadArgs = (struct thread_args){
                .sys = sys,
                .opcode = opcode,
                .isDebugEnabled = debugEnabled,
                .threadSync = threadSync
        };

        if (0 != (err = pthread_create(&timerThread, NULL, timerTick, &threadArgs))) {
                fprintf(stderr, "Couldn't create timerThread: errno(%d)\n", err);
        }

        if (0 != (err = pthread_create(&soundThread, NULL, soundWork, &threadArgs))) {
                fprintf(stderr, "Couldn't create soundThread: errno(%d)\n", err);
        }

        if (0 != (err = pthread_create(&gfxInputThread, NULL, gfxInputWork, &threadArgs))) {
                fprintf(stderr, "Couldn't create gfxInputThread: errno(%d)\n", err);
        }

        const double frequency = 1 / 500; // 500 FPS in MS per frame.

        while (!SystemShouldQuit(sys)) {
                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                if (SystemDebugIsEnabled(sys)) {
                        // debug ui
                        if (SystemDebugShouldFetchAndDecode(sys) && !SystemWFKWaiting(sys)) {
                                OpcodeFetch(opcode, sys);
                                OpcodeDecode(opcode);
                                SystemDecrementTimers(sys);
                                // Configure debug settings so UI input is required to proceed.
                                SystemDebugSetExecute(sys, 0);
                                SystemDebugSetFetchAndDecode(sys, 0);
                        }
                        else if (SystemWFKChanged(sys)) {
                                SystemIncrementPC(sys);
                                SystemWFKStop(sys);
                        }
                        else if (SystemDebugShouldExecute(sys)) {
                                OpcodeExecute(opcode, sys);
                                // Configure debug settings so we fetch the next instruction automatically.
                                SystemDebugSetExecute(sys, 0);
                                SystemDebugSetFetchAndDecode(sys, 1);
                        }
                } else {
                        // no debug ui
                        if (!SystemWFKWaiting(sys)) {
                                OpcodeFetch(opcode, sys);
                                OpcodeDecode(opcode);
                                SystemDecrementTimers(sys);
                        }
                        else if (SystemWFKChanged(sys)) {
                                SystemIncrementPC(sys);
                                SystemWFKStop(sys);
                        }
                        else {
                                OpcodeExecute(opcode, sys);
                        }
                }

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = (frequency - elapsed_time) * 1000 };
                nanosleep(&sleep, NULL);
        } // while (!SystemShouldQuit(sys))

        Shutdown(0);
}
