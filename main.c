/*
  Chip-8 Emulator (First Emulation Project)
*/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#define DISPLAY_WIDTH_WITH_DEBUGGER 1445
#define DISPLAY_HEIGHT_WITH_DEBUGGER 720

const unsigned int CHIP8_DISPLAY_WIDTH = 64;
const unsigned int CHIP8_DISPLAY_HEIGHT = 32;
const unsigned int DISPLAY_SCALE = 16;
unsigned int DISPLAY_WIDTH = 1445; // CHIP8_DISPLAY_WIDTH * DISPLAY_SCALE;
unsigned int DISPLAY_HEIGHT = 720; // CHIP8_DISPLAY_HEIGHT * DISPLAY_SCALE;

#include "input.h"
#include "opcode.h"
#include "sound.h"
#include "system.h"
#include "ui.h"

void Raster(struct system *s, GLubyte *texture) {
        for (int y = CHIP8_DISPLAY_HEIGHT-1, cy = 0; cy < CHIP8_DISPLAY_HEIGHT; cy++, y--) {
                for (int x = 0, cx = 0; cx < CHIP8_DISPLAY_WIDTH; cx++, x+=3) {
                        unsigned int pos = cy * (CHIP8_DISPLAY_WIDTH * 3) + x;

                        if (s->gfx[y * CHIP8_DISPLAY_WIDTH + cx]) {
                                // Black (Foreground)
                                texture[pos + 0] = 0;
                                texture[pos + 1] = 0;
                                texture[pos + 2] = 0;
                        } else {
                                // White (Background)
                                texture[pos + 0] = 0xFF;
                                texture[pos + 1] = 0xFF;
                                texture[pos + 2] = 0xFF;
                        }
                }
        }
}

// static const double MS_PER_FRAME = 0.03333333; // 30 FPS
static const double MS_PER_FRAME = 0.01666666; // 60 FPS
static int isDebugEnabled = 0;

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

void *timerTick(void *arguments) {
        struct system *system = (struct system *)&arguments[0];
        SystemDecrementTimers(system);
        return NULL;
}

int main(int argc, char **argv) {
        struct input *input = InputInit();
        struct opcode *opcode = OpcodeInit();
        struct sound *sound = SoundInit();
        struct system *system = SystemInit();

        GLubyte textureData[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT * 3];

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
        if (!SystemLoadProgram(system, mem, fsize)) {
                printf("Couldn't load program into Chip-8\n");
                goto quit;
        }

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

        if (isDebugEnabled) {
                DISPLAY_WIDTH = DISPLAY_WIDTH_WITH_DEBUGGER;
                DISPLAY_HEIGHT = DISPLAY_HEIGHT_WITH_DEBUGGER;
        } else {
                DISPLAY_WIDTH = CHIP8_DISPLAY_WIDTH * DISPLAY_SCALE;
                DISPLAY_HEIGHT = CHIP8_DISPLAY_HEIGHT * DISPLAY_SCALE;
        }

        SDL_Window *window = SDL_CreateWindow(
                "AaronO's CHIP-8 Emulator",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                DISPLAY_WIDTH, DISPLAY_HEIGHT,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
                );

        if (window == NULL) {
                printf("Couldn't open window: %s\n", SDL_GetError());
                goto quit;
        }

        SDL_GLContext glContext = SDL_GL_CreateContext(window);
        // TODO: Verify glContext is OK.
        int glWindowWidth, glWindowHeight;
        SDL_GetWindowSize(window, &glWindowWidth, &glWindowHeight);
        glViewport(0, 0, glWindowWidth, glWindowHeight);

        if (glewInit() != GLEW_OK) {
                fprintf(stderr, "Failed to setup GLEW\n");
                goto sdl_quit;
        }

        glEnable(GL_TEXTURE_2D);
        GLuint glTextureName;
        glGenTextures(1, &glTextureName);
        glBindTexture(GL_TEXTURE_2D, glTextureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)textureData);

        struct ui *ui = UIInit(isDebugEnabled, 240, 240, window);
        struct ui_debug *dbg = UIDebugInfo(ui);
        dbg->enabled = isDebugEnabled;

        SDL_Event event;
        int running = 1;
        while (running) {
                if (!dbg->enabled || (dbg->enabled && !dbg->waitForStep)) {
                        if (!SystemWFKWaiting(system)) {
                                OpcodeFetch(opcode, system);
                                OpcodeDecode(opcode, system);
                        }
                        SystemDecrementTimers(system);
                }

                if (dbg->enabled) {
                        dbg->waitForStep = 1;
                }

                UIInputBegin(ui); {
                        while (SDL_PollEvent(&event)) { // TODO: Causes SIGABRT?
                                running = InputCheck(input, system, &event);
                                UIHandleEvent(ui, &event);
                        }
                }
                UIInputEnd(ui);

                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = (MS_PER_FRAME - elapsed_time) * 1000 };
                //                nanosleep(&sleep, NULL);

                UIWidgets(ui, system, opcode);

                if (!SystemWFKWaiting(system)) {
                        if (SystemWFKChanged(system)) {
                                SystemIncrementPC(system);
                                SystemWFKStop(system);
                        }
                        else if (dbg->enabled && dbg->resume) {
                                dbg->resume = 0;
                                OpcodeExecute(opcode, system);
                        } else if (!dbg->enabled) {
                                OpcodeExecute(opcode, system);
                        }
                }

                SDL_GetWindowSize(window, &glWindowWidth, &glWindowHeight);
                glViewport(0, 0, glWindowWidth, glWindowHeight);
                glClear(GL_COLOR_BUFFER_BIT);
                glClearColor(0.10f, 0.18f, 0.24f, 1.0f);
                UIRender(ui);

                glOrtho(-1, 1, -1, 1, -1, 1);
                glColor3f(1, 1, 1);
                glEnable(GL_TEXTURE_2D);

                glBindTexture(GL_TEXTURE_2D, glTextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                Raster(system, textureData);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)textureData);

                float top, bottom, left, right;
                if (isDebugEnabled) {
                        top = 0.25;
                        bottom = -0.75;
                        left = 0;
                        right = 1;
                } else {
                        top = 1;
                        bottom = -1;
                        left = -1;
                        right = 1;
                }
                glBegin(GL_TRIANGLES); {
                        glTexCoord2f(0, 0);
                        glVertex3f(left, bottom, 0.5);
                        glTexCoord2f(0, 1);
                        glVertex3f(left, top, 0.5);
                        glTexCoord2f(1, 0);
                        glVertex3f(right, bottom, 0.5);

                        glTexCoord2f(0, 1);
                        glVertex3f(left, top, 0.5);
                        glTexCoord2f(1, 1);
                        glVertex3f(right, top, 0.5);
                        glTexCoord2f(1, 0);
                        glVertex3f(right, bottom, 0.5);
                } glEnd();

                SDL_GL_SwapWindow(window);
        } // while (running)

        SoundShutdown(sound);
        // nk_quit:
        UIShutdown(ui);
        // gl_quit:
        SDL_GL_DeleteContext(glContext); // TODO: Causes SIGABRT?
 sdl_quit:
        SDL_DestroyWindow(window); // TODO: Causes SIGABRT?
        SDL_Quit();
 quit:

        exit(0);
}
