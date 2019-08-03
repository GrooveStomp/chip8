/******************************************************************************
  File: ui.c
  Created: 2019-06-27
  Updated: 2019-07-30
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdlib.h> // malloc, free
#include <string.h> // memset

#include "ui.h"
#include "system.h"
#include "opcode.h"

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

struct ui {
        int enabled;
        struct nk_context *ctx;
        unsigned int widgetWidth;
        unsigned int widgetHeight;
        SDL_Window *window;
};

struct ui *UIInit(int shouldBeEnabled, unsigned int widgetWidth, unsigned int widgetHeight, SDL_Window *window) {
        struct ui *ui = (struct ui *)malloc(sizeof(struct ui));
        memset(ui, 0, sizeof(struct ui));

        ui->enabled = shouldBeEnabled;
        ui->widgetWidth = widgetWidth;
        ui->widgetHeight = widgetHeight;
        ui->window = window;

        if (ui->enabled) {
                ui->ctx = nk_sdl_init(ui->window); {
                        struct nk_font_atlas *atlas;
                        nk_sdl_font_stash_begin(&atlas);
                        nk_sdl_font_stash_end();
                }
        }

        return ui;
}

void UIInputBegin(struct ui *ui) {
        if (!ui->enabled) return;
        nk_input_begin(ui->ctx);
}

void UIInputEnd(struct ui *ui) {
        if (!ui->enabled) return;
        nk_input_end(ui->ctx);
}

void UIHandleEvent(struct ui *ui, SDL_Event *event) {
        if (!ui->enabled) return;
        nk_sdl_handle_event(event);
}


void UIWidgets(struct ui *ui, struct system *system, struct opcode *opcode) {
        if (!ui->enabled) return;

        if (nk_begin(ui->ctx, "Registers", nk_rect(0, 0, ui->widgetWidth, ui->widgetHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                static char textHexInput[16][64];
                static int textLength[16];

                for (int i = 0; i < 16; i++) {
                        sprintf(textHexInput[i], "0x%04X\n", system->v[i]);
                        textLength[i] = 16;
                }

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                for (int i = 0; i < 8; i++) {
                        // First 8
                        nk_layout_row_push(ui->ctx, 40);
                        nk_labelf(ui->ctx, NK_TEXT_RIGHT, "v%01X ", i);
                        nk_layout_row_push(ui->ctx, 60);
                        nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, textHexInput[i], &textLength[i], 64, nk_filter_hex);

                        // Second 8
                        nk_layout_row_push(ui->ctx, 40);
                        nk_labelf(ui->ctx, NK_TEXT_RIGHT, "v%01X ", i+8);
                        nk_layout_row_push(ui->ctx, 60);
                        nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, textHexInput[i+8], &textLength[i+8], 64, nk_filter_hex);
                }
                nk_layout_row_end(ui->ctx);
        }
        nk_end(ui->ctx);

        if (nk_begin(ui->ctx, "System", nk_rect(ui->widgetWidth, 0, ui->widgetWidth, ui->widgetHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
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
                sprintf(valDelayStr, "%02X", SystemDelayTimer(system));
                int delayTimerLen = 2;

                char valSoundStr[64];
                sprintf(valSoundStr, "%02X", SystemSoundTimer(system));
                int soundTimerLen = 2;

                char valFontPStr[64];
                sprintf(valFontPStr, "%04X", system->fontp);

                char valOpcodeStr[64];
                sprintf(valOpcodeStr, "%04X", OpcodeInstruction(opcode));

                int strLen = 4;

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "i");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valIStr, &strLen, 64, nk_filter_hex);
                nk_layout_row_end(ui->ctx);

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "pc");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valPcStr, &strLen, 64, nk_filter_hex);
                nk_layout_row_end(ui->ctx);

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "sp");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valSpStr, &strLen, 64, nk_filter_hex);
                nk_layout_row_end(ui->ctx);

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "gfx");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valGfxStr, &strLen, 64, nk_filter_hex);
                nk_layout_row_end(ui->ctx);

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "font ptr");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valFontPStr, &strLen, 64, nk_filter_hex);
                nk_layout_row_end(ui->ctx);

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "delay timer");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valDelayStr, &delayTimerLen, 64, nk_filter_decimal);
                nk_layout_row_end(ui->ctx);

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "sound timer");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valSoundStr, &soundTimerLen, 64, nk_filter_decimal);
                nk_layout_row_end(ui->ctx);

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                nk_layout_row_push(ui->ctx, labelWidth);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "opcode");
                nk_layout_row_push(ui->ctx, valueWidth);
                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, valOpcodeStr, &strLen, 64, nk_filter_hex);
                nk_layout_row_end(ui->ctx);
        }
        nk_end(ui->ctx);

        if (nk_begin(ui->ctx, "Stack", nk_rect(ui->widgetWidth * 2, 0, ui->widgetWidth, ui->widgetHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                static char textHexInput[16][64];
                static int textLength[16];

                for (int i = 0; i < 16; i++) {
                        sprintf(textHexInput[i], "0x%04X\n", system->stack[i]);
                        textLength[i] = 16;
                }

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                for (int i = 0; i < 8; i++) {
                        // First 8
                        nk_layout_row_push(ui->ctx, 40);
                        nk_labelf(ui->ctx, NK_TEXT_RIGHT, "s%01X ", i);
                        nk_layout_row_push(ui->ctx, 60);
                        nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, textHexInput[i], &textLength[i], 64, nk_filter_hex);

                        // Second 8
                        nk_layout_row_push(ui->ctx, 40);
                        nk_labelf(ui->ctx, NK_TEXT_RIGHT, "s%01X ", i+8);
                        nk_layout_row_push(ui->ctx, 60);
                        nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, textHexInput[i+8], &textLength[i+8], 64, nk_filter_hex);
                }
        }
        nk_end(ui->ctx);

        if (nk_begin(ui->ctx, "Memory", nk_rect(0, ui->widgetHeight, ui->widgetWidth * 3, ui->widgetHeight * 2), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                /*
                  | HEX | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | BYTES         |
                  |-----+-------------------------------------------------+---------------|
                  | 000 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | eR?NTFS    .. |
                  | 010 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | .....|..?.|.. |
                */
                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 18);
                nk_layout_row_push(ui->ctx, 40);
                nk_labelf(ui->ctx, NK_TEXT_RIGHT, "HEX |");
                for (int i = 0; i < 16; i++) {
                        nk_layout_row_push(ui->ctx, 25);
                        nk_labelf(ui->ctx, NK_TEXT_CENTERED, "%02X", i);
                }
                nk_layout_row_push(ui->ctx, 80);
                nk_labelf(ui->ctx, NK_TEXT_LEFT, "| BYTES");
                nk_layout_row_end(ui->ctx);

                for (int i = 0; i < 4096; i+=16) {
                        nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 19);
                        nk_layout_row_push(ui->ctx, 40);
                        nk_labelf(ui->ctx, NK_TEXT_CENTERED, "%04X |", i);

                        for (int j = 0; j < 16; j++) {
                                char text[4];
                                int textLen = 2;
                                snprintf(text, textLen + 1, "%02X", system->memory[i + j]);
                                nk_layout_row_push(ui->ctx, 25);
                                nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, text, &textLen, 64, nk_filter_hex);
                        }

                        nk_layout_row_push(ui->ctx, 10);
                        nk_labelf(ui->ctx, NK_TEXT_LEFT, "| ");
                        nk_layout_row_push(ui->ctx, 100);
                        char text[17] = { 0 };
                        int textLen = 16;
                        snprintf(text, textLen + 1, "%s", &system->memory[i]);
                        nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, text, &textLen, 64, nk_filter_ascii);
                        nk_layout_row_end(ui->ctx);
                }
        }
        nk_end(ui->ctx);

        if (nk_begin(ui->ctx, "Debugger", nk_rect(ui->widgetWidth * 3, 0, ui->widgetWidth, ui->widgetHeight / 2.0), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                nk_layout_row_static(ui->ctx, 20, ui->widgetWidth - 40, 1);
                if (nk_button_label(ui->ctx, "Step")) {
                        SystemDebugSetFetchAndDecode(system, 0);
                        SystemDebugSetExecute(system, 1);
                }
                if (nk_button_label(ui->ctx, "Continue")) {
                        SystemDebugSetEnabled(system, 0);
                }
                if (nk_button_label(ui->ctx, "Break")) {
                        SystemDebugSetFetchAndDecode(system, 1);
                        SystemDebugSetEnabled(system, 1);
                }
        }
        nk_end(ui->ctx);

        if (nk_begin(ui->ctx, "Opcode", nk_rect(ui->widgetWidth * 3, ui->widgetHeight / 2.0, ui->widgetWidth, ui->widgetHeight / 2.0), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                char desc[256] = { 0 };
                OpcodeDescription(opcode, desc, 256);
                nk_layout_row_dynamic(ui->ctx, 20, 1);
                nk_labelf(ui->ctx, NK_TEXT_LEFT, "%04X", OpcodeInstruction(opcode));

                nk_layout_row_dynamic(ui->ctx, 100, 1);
                nk_label_wrap(ui->ctx, (const char *)desc);
        }
        nk_end(ui->ctx);

        if (nk_begin(ui->ctx, "Keys", nk_rect(ui->widgetWidth * 4, 0, ui->widgetWidth, ui->widgetHeight), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
                static char textHexInput[16][64];
                static int textLength[16];

                for (int i = 0; i < 16; i++) {
                        if (SystemKeyIsPressed(system, i)) {
                                sprintf(textHexInput[i], "PRESSED");
                                textLength[i] = 7;
                        } else {
                                textHexInput[i][0] = '\0';
                                textLength[i] = 0;
                        }

                }

                nk_layout_row_begin(ui->ctx, NK_STATIC, 20, 4);
                for (int i = 0; i < 8; i++) {
                        // First 8
                        nk_layout_row_push(ui->ctx, 40);
                        nk_labelf(ui->ctx, NK_TEXT_RIGHT, "k%01X ", i);
                        nk_layout_row_push(ui->ctx, 60);
                        nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, textHexInput[i], &textLength[i], 64, nk_filter_hex);

                        // Second 8
                        nk_layout_row_push(ui->ctx, 40);
                        nk_labelf(ui->ctx, NK_TEXT_RIGHT, "k%01X ", i+8);
                        nk_layout_row_push(ui->ctx, 60);
                        nk_edit_string(ui->ctx, NK_EDIT_SIMPLE, textHexInput[i+8], &textLength[i+8], 64, nk_filter_hex);
                }
        }
        nk_end(ui->ctx);
}

void UIRender(struct ui *ui) {
        if (!ui->enabled) return;

        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
}

void UIDeinit(struct ui *ui) {
        if (NULL == ui)
                return;

        if (ui->enabled) {
                nk_sdl_shutdown();
        }

        free(ui);
}
