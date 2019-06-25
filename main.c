/*
  Chip-8 Emulator (First Emulation Project)
*/
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "external/nuklear.h"
#include "external/nuklear_sdl_gl3.h"

#define MAX_VERTEX_MEMORY 512 * 1024 // ??
#define MAX_ELEMENT_MEMORY 128 * 1024 // ??

const unsigned int CHIP8_DISPLAY_WIDTH = 64;
const unsigned int CHIP8_DISPLAY_HEIGHT = 32;
const unsigned int DISPLAY_SCALE = 16;
const unsigned int DISPLAY_WIDTH = 1445; // CHIP8_DISPLAY_WIDTH * DISPLAY_SCALE;
const unsigned int DISPLAY_HEIGHT = 720; // CHIP8_DISPLAY_HEIGHT * DISPLAY_SCALE;

#include "system.h"
#include "opcode.h"
#include "input.h"

void Raster(struct system *s, GLubyte *texture) {
        for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
                for (int x = 0, cx = 0; cx < CHIP8_DISPLAY_WIDTH; cx++, x+=3) {
                        unsigned int pos = y * (CHIP8_DISPLAY_WIDTH * 3) + x;

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
static int DEBUG_MODE = 0;
static int DEBUG_CONTINUE = 0;
static int DEBUG_WAIT_FOR_STEP = 0;

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
        }

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

        SDL_Window *window = SDL_CreateWindow(
                "Window Title",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                DISPLAY_WIDTH, DISPLAY_HEIGHT,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
                );

        int running = 1;
        if (window == NULL) {
                printf("Couldn't open window: %s\n", SDL_GetError());
                running = 0;
        }

        SDL_GLContext glContext = SDL_GL_CreateContext(window);
        int glWindowWidth, glWindowHeight;
        SDL_GetWindowSize(window, &glWindowWidth, &glWindowHeight);
        glViewport(0, 0, glWindowWidth, glWindowHeight);

        if (glewInit() != GLEW_OK) {
                fprintf(stderr, "Failed to setup GLEW\n");
                exit(1);
        }

        GLubyte textureData[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT * 3];
        glEnable(GL_TEXTURE_2D);
        GLuint glTextureName;
        glGenTextures(1, &glTextureName);
        glBindTexture(GL_TEXTURE_2D, glTextureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)textureData);

        struct nk_context *ctx = nk_sdl_init(window); {
                struct nk_font_atlas *atlas;
                nk_sdl_font_stash_begin(&atlas);
                nk_sdl_font_stash_end();
        }

        struct nk_colorf bg = (struct nk_colorf){ .r = 0.10f, .g = 0.18f, .b = 0.24f, .a = 1.0f };

        unsigned int widgetWidth = 240;
        unsigned int widgetHeight = 240;

        while (running) {
                if (!DEBUG_MODE || (DEBUG_MODE && !DEBUG_WAIT_FOR_STEP)) {
                        if (system->waitForKey == -1) {
                                OpcodeFetch(opcode, system);
                                OpcodeDecode(opcode, system);
                        }

                        SystemDecrementTimers(system);
                        SystemClearKeys(system);

                }

                if (DEBUG_MODE) {
                        DEBUG_WAIT_FOR_STEP = 1;
                }

                nk_input_begin(ctx); {
                        SDL_Event event;
                        while (SDL_PollEvent(&event)) {
                                running = InputCheck(input, system, &event);
                                nk_sdl_handle_event(&event);
                        }
                }
                nk_input_end(ctx);

                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = (MS_PER_FRAME - elapsed_time) * 1000 };
                nanosleep(&sleep, NULL);

                if (nk_begin(ctx, "Registers", nk_rect(0, 0, widgetWidth, widgetHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                        static char textHexInput[16][64];
                        static int textLength[16];

                        for (int i = 0; i < 16; i++) {
                                sprintf(textHexInput[i], "0x%04X\n", system->v[i]);
                                textLength[i] = 16;
                        }

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        for (int i = 0; i < 8; i++) {
                                // First 8
                                nk_layout_row_push(ctx, 40);
                                nk_labelf(ctx, NK_TEXT_RIGHT, "v%01X ", i);
                                nk_layout_row_push(ctx, 60);
                                nk_edit_string(ctx, NK_EDIT_SIMPLE, textHexInput[i], &textLength[i], 64, nk_filter_hex);

                                // Second 8
                                nk_layout_row_push(ctx, 40);
                                nk_labelf(ctx, NK_TEXT_RIGHT, "v%01X ", i+8);
                                nk_layout_row_push(ctx, 60);
                                nk_edit_string(ctx, NK_EDIT_SIMPLE, textHexInput[i+8], &textLength[i+8], 64, nk_filter_hex);
                        }
                        nk_layout_row_end(ctx);
                }
                nk_end(ctx);

                if (nk_begin(ctx, "System", nk_rect(widgetWidth, 0, widgetWidth, widgetHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                        int labelWidth = 80;
                        int valueWidth = 60;

                        char valIStr[64];
                        sprintf(valIStr, "%04X", system->i);

                        char valPcStr[64];
                        sprintf(valPcStr, "%04X", system->pc);

                        char valSpStr[64];
                        sprintf(valSpStr, "%04X", system->sp);

                        char valGfxStr[64];
                        sprintf(valGfxStr, "%p", (void *)system->gfx);

                        char valDelayStr[64];
                        sprintf(valDelayStr, "%02X", system->delayTimer);
                        int delayTimerLen = 2;

                        char valSoundStr[64];
                        sprintf(valSoundStr, "%02X", system->soundTimer);
                        int soundTimerLen = 2;

                        char valFontPStr[64];
                        sprintf(valFontPStr, "%04X", system->fontp);

                        char valOpcodeStr[64];
                        sprintf(valOpcodeStr, "%04X", OpcodeInstruction(opcode));

                        int strLen = 4;

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "i");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valIStr, &strLen, 64, nk_filter_hex);
                        nk_layout_row_end(ctx);

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "pc");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valPcStr, &strLen, 64, nk_filter_hex);
                        nk_layout_row_end(ctx);

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "sp");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valSpStr, &strLen, 64, nk_filter_hex);
                        nk_layout_row_end(ctx);

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "gfx");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valGfxStr, &strLen, 64, nk_filter_hex);
                        nk_layout_row_end(ctx);

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "font ptr");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valFontPStr, &strLen, 64, nk_filter_hex);
                        nk_layout_row_end(ctx);

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "delay timer");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valDelayStr, &delayTimerLen, 64, nk_filter_decimal);
                        nk_layout_row_end(ctx);

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "sound timer");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valSoundStr, &soundTimerLen, 64, nk_filter_decimal);
                        nk_layout_row_end(ctx);

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        nk_layout_row_push(ctx, labelWidth);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "opcode");
                        nk_layout_row_push(ctx, valueWidth);
                        nk_edit_string(ctx, NK_EDIT_SIMPLE, valOpcodeStr, &strLen, 64, nk_filter_hex);
                        nk_layout_row_end(ctx);
                }
                nk_end(ctx);

                if (nk_begin(ctx, "Stack", nk_rect(widgetWidth * 2, 0, widgetWidth, widgetHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                        static char textHexInput[16][64];
                        static int textLength[16];

                        for (int i = 0; i < 16; i++) {
                                sprintf(textHexInput[i], "0x%04X\n", system->stack[i]);
                                textLength[i] = 16;
                        }

                        nk_layout_row_begin(ctx, NK_STATIC, 20, 4);
                        for (int i = 0; i < 8; i++) {
                                // First 8
                                nk_layout_row_push(ctx, 40);
                                nk_labelf(ctx, NK_TEXT_RIGHT, "s%01X ", i);
                                nk_layout_row_push(ctx, 60);
                                nk_edit_string(ctx, NK_EDIT_SIMPLE, textHexInput[i], &textLength[i], 64, nk_filter_hex);

                                // Second 8
                                nk_layout_row_push(ctx, 40);
                                nk_labelf(ctx, NK_TEXT_RIGHT, "s%01X ", i+8);
                                nk_layout_row_push(ctx, 60);
                                nk_edit_string(ctx, NK_EDIT_SIMPLE, textHexInput[i+8], &textLength[i+8], 64, nk_filter_hex);
                        }
                }
                nk_end(ctx);

                if (nk_begin(ctx, "Memory", nk_rect(0, widgetHeight, widgetWidth * 3, widgetHeight * 2), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                        /*
                          | HEX | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | BYTES         |
                          |-----+-------------------------------------------------+---------------|
                          | 000 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | eR?NTFS    .. |
                          | 010 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | .....|..?.|.. |
                         */
                        nk_layout_row_begin(ctx, NK_STATIC, 20, 18);
                        nk_layout_row_push(ctx, 40);
                        nk_labelf(ctx, NK_TEXT_RIGHT, "HEX |");
                        for (int i = 0; i < 16; i++) {
                                nk_layout_row_push(ctx, 25);
                                nk_labelf(ctx, NK_TEXT_CENTERED, "%02X", i);
                        }
                        nk_layout_row_push(ctx, 80);
                        nk_labelf(ctx, NK_TEXT_LEFT, "| BYTES");
                        nk_layout_row_end(ctx);

                        for (int i = 0; i < 4096; i+=16) {
                                nk_layout_row_begin(ctx, NK_STATIC, 20, 19);
                                nk_layout_row_push(ctx, 40);
                                nk_labelf(ctx, NK_TEXT_CENTERED, "%04X |", i);

                                for (int j = 0; j < 16; j++) {
                                        char text[2];
                                        int textLen = 2;
                                        snprintf(text, textLen + 1, "%02X", system->memory[i + j]);
                                        nk_layout_row_push(ctx, 25);
                                        nk_edit_string(ctx, NK_EDIT_SIMPLE, text, &textLen, 64, nk_filter_hex);
                                }

                                nk_layout_row_push(ctx, 10);
                                nk_labelf(ctx, NK_TEXT_LEFT, "| ");
                                nk_layout_row_push(ctx, 100);
                                char text[16];
                                int textLen = 16;
                                snprintf(text, textLen + 1, "%s", (char *)&system->memory[i]);
                                nk_edit_string(ctx, NK_EDIT_SIMPLE, text, &textLen, 64, nk_filter_default);
                                nk_layout_row_end(ctx);
                        }
                }
                nk_end(ctx);

                if (nk_begin(ctx, "Debugger", nk_rect(widgetWidth * 3, 0, widgetWidth, widgetHeight / 2.0), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                        nk_layout_row_static(ctx, 20, widgetWidth - 40, 1);
                        if (nk_button_label(ctx, "Step")) {
                                if (DEBUG_MODE) {
                                        DEBUG_CONTINUE = 1;
                                        DEBUG_WAIT_FOR_STEP = 0;
                                }
                        }
                        if (nk_button_label(ctx, "Continue")) {
                                DEBUG_MODE = 0;
                                DEBUG_CONTINUE = 1;
                                DEBUG_WAIT_FOR_STEP = 1;
                        }
                        if (nk_button_label(ctx, "Break")) {
                                DEBUG_MODE = 1;
                                DEBUG_CONTINUE = 1;
                                DEBUG_WAIT_FOR_STEP = 1;
                        }
                }
                nk_end(ctx);

                if (nk_begin(ctx, "Opcode", nk_rect(widgetWidth * 3, widgetHeight / 2.0, widgetWidth, widgetHeight / 2.0), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                        char *desc = OpcodeDescription(opcode);
                        nk_layout_row_dynamic(ctx, 20, 1);
                        nk_labelf(ctx, NK_TEXT_LEFT, "%04X", OpcodeInstruction(opcode));

                        if (desc != NULL) {
                                nk_layout_row_dynamic(ctx, 100, 1);
                                nk_label_wrap(ctx, desc);
                                OpcodeFree(desc);
                        }
                }
                nk_end(ctx);

                if (system->waitForKey == -1) {
                        if (DEBUG_MODE && DEBUG_CONTINUE) {
                                DEBUG_CONTINUE = 0;
                                OpcodeExecute(opcode, system);
                        } else if (!DEBUG_MODE) {
                                OpcodeExecute(opcode, system);
                        }
                }

                SDL_GetWindowSize(window, &glWindowWidth, &glWindowHeight);
                glViewport(0, 0, glWindowWidth, glWindowHeight);
                glClear(GL_COLOR_BUFFER_BIT);
                glClearColor(bg.r, bg.g, bg.b, bg.a);
                nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

                glOrtho(-1, 1, -1, 1, -1, 1);
                glColor3f(1, 1, 1);
                glEnable(GL_TEXTURE_2D);

                glBindTexture(GL_TEXTURE_2D, glTextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                Raster(system, textureData);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)textureData);

                float top = 0.25;
                float bottom = -0.75;
                float left = 0;
                float right = 1;
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

        //cleanup:
        nk_sdl_shutdown();
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();

        exit(0);
}
