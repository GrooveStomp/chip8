/*
  Chip-8 Emulator (First Emulation Project)
*/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "SDL.h"

const unsigned int CHIP8_WIDTH = 64;
const unsigned int CHIP8_HEIGHT = 32;
const unsigned int DISPLAY_SCALE = 16;
const unsigned int DISPLAY_WIDTH = CHIP8_WIDTH * DISPLAY_SCALE;
const unsigned int DISPLAY_HEIGHT = CHIP8_HEIGHT * DISPLAY_SCALE;

#include "system.h"
#include "opcode.h"
#include "input.h"

void Embiggen(unsigned int *buf, unsigned int x, unsigned int y, unsigned int pixel) {
        for (int sy = y; sy < y + DISPLAY_SCALE; sy++) {
                for (int sx = x; sx < x + DISPLAY_SCALE; sx++) {
                        buf[sy * DISPLAY_WIDTH + sx] = pixel;
                }
        }
}

void Raster(struct system *s, SDL_Texture *texture) {
        unsigned int *buf;
        int pitch;

        if (SDL_LockTexture(texture, NULL, (void **)&buf, &pitch) != 0) {
                printf("Couldn't lock texture: %s\n", SDL_GetError());
                return;
        }

        for (int y = 0, cy = 0; y < DISPLAY_HEIGHT; y+=DISPLAY_SCALE, cy++) {
                for (int x = 0, cx = 0; x < DISPLAY_WIDTH; x+=DISPLAY_SCALE, cx++) {
                        if (s->gfx[cy * CHIP8_WIDTH + cx]) {
                                // Black (Foreground)
                                Embiggen(buf, x, y, 0x000000FF);
                        } else {
                                // White (Background)
                                Embiggen(buf, x, y, 0xFFFFFFFF);
                        }
                }
        }

        SDL_UnlockTexture(texture);
}

// static const double MS_PER_FRAME = 0.03333333; // 30 FPS
static const double MS_PER_FRAME = 0.01666666; // 60 FPS
static int DEBUG_MODE = 0;
static char *PROGRAM = NULL;

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
                DEBUG_MODE = 1;
        } else if (argc == 3 && strcmp(argv[2], "-d") == 0) {
                DEBUG_MODE = 1;
        } else if (argc == 3) {
                Usage();
                exit(1);
        }
}

int main(int argc, char **argv) {
        struct opcode *opcode = OpcodeInit();
        struct system *system = SystemInit();
        struct input *input = InputInit();

        ArgParse(argc, argv, 0);

        size_t fsize = 0;
        char *mem;
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
                mem = malloc(fsize);
                while ((c = fgetc(f)) != EOF) {
                        mem[i] = (char)c;
                        i++;
                }

                fclose(f);
        }
        if (!SystemLoadProgram(system, mem, fsize)) {
                printf("Couldn't load program into Chip-8\n");
        }

        SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

        SDL_Window *window = SDL_CreateWindow(
                "Window Title",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                DISPLAY_WIDTH, DISPLAY_HEIGHT,
                0);
        if (window == NULL) {
                printf("Couldn't open window: %s\n", SDL_GetError());
                running = 0;
        }

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        if (renderer == NULL) {
                printf("Couldn't create renderer: %s\n", SDL_GetError());
                running = 0;
        }

        SDL_Texture *texture = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_STREAMING,
                DISPLAY_WIDTH,
                DISPLAY_HEIGHT);
        if (texture == NULL) {
                printf("Couldn't create texture: %s\n", SDL_GetError());
                running = 0;
        }

        int running = 1;
        while (running) {
                if (system->waitForKey == -1) {
                        OpcodeFetch(opcode, system);
                        OpcodeDecode(opcode, system);
                        if (DEBUG_MODE) {
                                OpcodeDebug(opcode);
                                SystemDebug(system);
                                char in[256];
                                fgets(in, 256, stdin);
                        }
                        OpcodeExecute(opcode, system);
                }

                SystemDecrementTimers(system);
                SystemClearKeys(system);

                running = !InputCheck(input, system);

                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                if (!DEBUG_MODE) {
                        struct timespec end;
                        clock_gettime(CLOCK_REALTIME, &end);

                        double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                        elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms

                        struct timespec sleep = { .tv_sec = 0, .tv_nsec = (MS_PER_FRAME - elapsed_time) * 1000 };
                        nanosleep(&sleep, NULL);
                }

                Raster(system, texture);

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, 0, 0);
                SDL_RenderPresent(renderer);
        } // while (running)

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
}
