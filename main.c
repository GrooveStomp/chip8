/******************************************************************************
  File: main.c
  Created: 2019-06-04
  Updated: 2019-07-16
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
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

struct sound_thread_args {
        struct sound *sound;
        struct system *sys;
};

static int isDebugEnabled = 0;

static struct graphics *graphics;
static struct input *input;
static struct opcode *opcode;
static struct sound *sound;
static struct system *sys;
static struct ui *ui;

// TODO: Need to terminate this thread so pthread_join works.
void *timerTick(void *context) {
        static const double msPerFrame = 0.01666666; // 60 FPS

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
        struct system *sys = (struct system *)context;
#pragma GCC diagnostic pop
        for (;;) {
                if (UIDebugIsWaiting(ui)) {
                        continue;
                }

                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                SystemDecrementTimers(sys);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = (msPerFrame - elapsed_time) * 1000 };
                nanosleep(&sleep, NULL);
        }

        return NULL;
}

// TODO: Need to terminate this thread so pthread_join works.
void *soundWork(void *ctx) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
        struct sound_thread_args *context = (struct sound_thread_args *)ctx;
#pragma GCC diagnostic pop

        struct timer *timer = TimerInit(200);
        int playing = 0;
        for (;;) {
                if (SystemSoundTriggered(context->sys)) {
                        TimerReset(timer);
                        SystemSoundSetTrigger(context->sys, 0);
                        playing = 1;
                        SoundPlay(context->sound);
                }

                if (playing && TimerHasElapsed(timer)) {
                        SoundStop(context->sound);
                        playing = 0;
                }
        }

        return NULL;
}

void UIRenderFn() {
        UIRender(ui);
}

void Shutdown(int status) {
        if (NULL != ui)
                UIDeinit(ui);

        if (NULL != graphics)
                GraphicsDeinit(graphics);

        if (NULL != sound)
                SoundDeinit(sound);

        if (NULL != input)
                InputDeinit(input);

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

void ArgParse(int argc, char **argv, int debug) {
        if (debug) {
                printf("argc: %d, argv: ", argc);
                for (int i=0; i< argc; i++) {
                        printf("%s ", argv[i]);
                }
                printf("\n");
        }

        if (argc < 2 || argc > 3) {
                Usage();
                exit(1);
        }

        if (argc == 3 && strcmp(argv[1], "-d") == 0) {
                argv[1] = argv[2];
                isDebugEnabled = 1;
        } else if (argc == 3 && strcmp(argv[2], "-d") == 0) {
                isDebugEnabled = 1;
        } else if (argc == 3) {
                Usage();
                exit(1);
        }
}

int main(int argc, char **argv) {
        ArgParse(argc, argv, 0);

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

        sys = SystemInit();
        if (!SystemLoadProgram(sys, mem, fsize)) {
                fprintf(stderr, "Couldn't load program into Chip-8\n");
                Shutdown(1);
        }

        opcode = OpcodeInit();
        input = InputInit();
        sound = SoundInit();
        graphics = GraphicsInit(isDebugEnabled);

        ui = UIInit(isDebugEnabled, 240, 240, GraphicsSDLWindow(graphics));
        if (ui == NULL) {
                fprintf(stderr, "Couldn't initialize ui\n");
                Shutdown(1);
        }
        UIDebugSetEnabled(ui, isDebugEnabled);

        int err;
        pthread_t timersThread;
        if (0 != (err = pthread_create(&timersThread, NULL, timerTick, sys))) {
                fprintf(stderr, "Couldn't create timer thread: errno(%d)\n", err);
        }

        pthread_t soundThread;
        struct sound_thread_args args = { .sound = sound, .sys = sys };
        if (0 != (err = pthread_create(&soundThread, NULL, soundWork, (void *)&args))) {
                fprintf(stderr, "Couldn't create timer thread: errno(%d)\n", err);
        }

        SDL_Event event;
        int running = 1;
        while (running) {
                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                struct ui_debug uiDbg = UIDebugInfo(ui);
                if (!uiDbg.enabled || (uiDbg.enabled && !uiDbg.waiting)) {
                        if (!SystemWFKWaiting(sys)) {
                                OpcodeFetch(opcode, sys);
                                OpcodeDecode(opcode, sys);
                                SystemDecrementTimers(sys);
                        }
                }

                if (uiDbg.enabled) {
                        UIDebugSetWaiting(ui, 1);
                }

                UIInputBegin(ui); {
                        while (SDL_PollEvent(&event)) { // TODO: Causes SIGABRT?
                                running = InputCheck(input, sys, &event);
                                UIHandleEvent(ui, &event);
                        }
                }
                UIInputEnd(ui);

                UIWidgets(ui, sys, opcode);

                if (!SystemWFKWaiting(sys)) {
                        struct ui_debug uiDbg = UIDebugInfo(ui);
                        if (SystemWFKChanged(sys)) {
                                SystemIncrementPC(sys);
                                SystemWFKStop(sys);
                        } else if (uiDbg.enabled && uiDbg.resume) {
                                UIDebugSetResume(ui, 0);
                                OpcodeExecute(opcode, sys);
                        } else if (!uiDbg.enabled) {
                                OpcodeExecute(opcode, sys);
                        }
                }

                GraphicsPresent(graphics, sys, UIRenderFn);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms
        } // while (running)

        Shutdown(0);
}
