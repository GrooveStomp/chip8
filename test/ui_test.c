/******************************************************************************
  File: ui_test.c
  Created: 2019-08-05
  Updated: 2019-08-06
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
// Disables warnings on some sloppy string formatting usage. ie., function
// pointer instead of void pointer.
#pragma GCC diagnostic ignored "-Wformat="

#include <dlfcn.h> // dlsym, RTLD_NEXT
#include <stdio.h>

#include "gstest.h"
int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

#include "../ui.h"
#include "../ui.c"
#include "../graphics.h"

//------------------------------------------------------------------------------
// Helper functions and globals
//------------------------------------------------------------------------------
int customFreeCount = 0;
int useCustomFree = 0;

void free(void *p) {
        static void (*originalFn)(void *) = NULL;
        if (NULL == originalFn) {
                *(void **)(&originalFn) = dlsym(RTLD_NEXT, "free");
        }

        if (useCustomFree) {
                customFreeCount++;
        }

        originalFn(p);
}

//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------
static char *TestUIInitDebug() {
        struct graphics *graphics = GraphicsInit(1);
        if (NULL == graphics) {
                GSTestAssert(0, "Couldn't initialize graphics");
        }

        SDL_Window *window = GraphicsSDLWindow(graphics);
        struct ui *ui = UIInit(1, 25, 26, window);
        if (NULL == ui) {
                GSTestAssert(0, "Couldn't initialize ui");
        }

        GSTestAssert(ui->enabled == 1, "got %d, want %d", ui->enabled, 1);
        GSTestAssert(ui->widgetWidth == 25, "got %d, want %d", ui->widgetWidth, 25);
        GSTestAssert(ui->widgetHeight == 26, "got %d, want %d", ui->widgetHeight, 26);
        GSTestAssert(ui->window == window, "got %p, want %p", ui->window, window);

        // Now verify the sdl window is set.
        GSTestAssert(sdl.win == window, "got %p, want %p", sdl.win, window);

        UIDeinit(ui);
        GraphicsDeinit(graphics);

        return NULL;
}

static char *TestUIInitNoDebug() {
        struct graphics *graphics = GraphicsInit(0);
        if (NULL == graphics) {
                GSTestAssert(0, "Couldn't initialize graphics");
        }

        SDL_Window *window = GraphicsSDLWindow(graphics);
        struct ui *ui = UIInit(0, 25, 26, window);
        if (NULL == ui) {
                GSTestAssert(0, "Couldn't initialize ui");
        }

        GSTestAssert(ui->enabled == 0, "got %d, want %d", ui->enabled, 0);
        GSTestAssert(ui->widgetWidth == 25, "got %d, want %d", ui->widgetWidth, 25);
        GSTestAssert(ui->widgetHeight == 26, "got %d, want %d", ui->widgetHeight, 26);
        GSTestAssert(ui->window == window, "got %p, want %p", ui->window, window);

        // Now verify the sdl window was _not_ set, due to debugging being disabled.
        GSTestAssert(sdl.win == NULL, "got %p, want %p", sdl.win, NULL);

        UIDeinit(ui);
        GraphicsDeinit(graphics);

        return NULL;
}

static char *TestUIDeinit() {
        SDL_Window *window = NULL;
        struct ui *ui = UIInit(0, 25, 26, window);

        int before = customFreeCount;
        useCustomFree = 1;
        UIDeinit(ui);
        useCustomFree = 0;
        GSTestAssert(customFreeCount > before, "got %d, want more than %d", customFreeCount, before);

        return NULL;
}

static char *TestUIInputBegin() {
        struct graphics *graphics = GraphicsInit(1);
        if (NULL == graphics) {
                GSTestAssert(0, "Couldn't initialize graphics");
        }

        SDL_Window *window = GraphicsSDLWindow(graphics);
        struct ui *ui = UIInit(1, 25, 26, window);
        if (NULL == ui) {
                GSTestAssert(0, "Couldn't initialize ui");
        }

        // Set up some internal state we can assert on.
        ui->ctx->input.keyboard.text_len = 5;
        ui->ctx->input.keyboard.keys[0].clicked = 1;

        // Now disable debugging and verify no changes.
        {
                ui->enabled = 0;
                UIInputBegin(ui);

                GSTestAssert(ui->ctx->input.keyboard.text_len == 5, "got %d, want %d", ui->ctx->input.keyboard.text_len, 5);
                GSTestAssert(ui->ctx->input.keyboard.keys[0].clicked == 1, "got %d, want %d", ui->ctx->input.keyboard.keys[0].clicked, 1);
        }

        // Now enable debugging and verify changes.
        {
                ui->enabled = 1;
                UIInputBegin(ui);

                GSTestAssert(ui->ctx->input.keyboard.text_len == 0, "got %d, want %d", ui->ctx->input.keyboard.text_len, 0);
                GSTestAssert(ui->ctx->input.keyboard.keys[0].clicked == 0, "got %d, want %d", ui->ctx->input.keyboard.keys[0].clicked, 0);
        }

        UIDeinit(ui);
        GraphicsDeinit(graphics);

        return NULL;
}

static char *TestUIInputEnd() {
        struct graphics *graphics = GraphicsInit(1);
        if (NULL == graphics) {
                GSTestAssert(0, "Couldn't initialize graphics");
        }

        SDL_Window *window = GraphicsSDLWindow(graphics);
        struct ui *ui = UIInit(1, 25, 26, window);
        if (NULL == ui) {
                GSTestAssert(0, "Couldn't initialize ui");
        }

        // Set up some internal state we can assert on.
        ui->ctx->input.mouse.grab = 1;

        // Now disable debugging and verify no changes.
        {
                ui->enabled = 0;
                UIInputEnd(ui);
                GSTestAssert(ui->ctx->input.mouse.grab == 1, "got %d, want %d", ui->ctx->input.mouse.grab, 1);
        }

        // Now enable debugging and verify changes.
        {
                ui->enabled = 1;
                UIInputEnd(ui);
                GSTestAssert(ui->ctx->input.mouse.grab == 0, "got %d, want %d", ui->ctx->input.mouse.grab, 0);
        }

        UIDeinit(ui);
        GraphicsDeinit(graphics);

        return NULL;
}

static char *TestUIHandleEvent() {
        struct graphics *graphics = GraphicsInit(1);
        if (NULL == graphics) {
                GSTestAssert(0, "Couldn't initialize graphics");
        }

        SDL_Window *window = GraphicsSDLWindow(graphics);
        struct ui *ui = UIInit(1, 25, 26, window);
        if (NULL == ui) {
                GSTestAssert(0, "Couldn't initialize ui");
        }

        // Set up some internal state we can assert on.
        ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].down = 0;
        ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].clicked = 0;

        SDL_Event event = { .type = SDL_KEYDOWN };
        event.key.keysym.sym = SDLK_BACKSPACE;

        // Now disable debugging and verify no changes.
        {
                ui->enabled = 0;
                UIHandleEvent(ui, &event);
                GSTestAssert(ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].down == 0, "got %d, want %d", ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].down, 0);
                GSTestAssert(ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].clicked == 0, "got %d, want %d", ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].clicked, 0);
        }

        // Now enable debugging and verify changes.
        {
                ui->enabled = 1;
                UIHandleEvent(ui, &event);
                GSTestAssert(ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].down == 1, "got %d, want %d", ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].down, 1);
                GSTestAssert(ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].clicked == 1, "got %d, want %d", ui->ctx->input.keyboard.keys[NK_KEY_BACKSPACE].clicked, 1);
        }

        UIDeinit(ui);
        GraphicsDeinit(graphics);

        return NULL;
}

static char *RunAllTests() {
        GSTestRun(TestUIInitDebug);
        GSTestRun(TestUIInitNoDebug);
        GSTestRun(TestUIDeinit);
        GSTestRun(TestUIInputBegin);
        GSTestRun(TestUIInputEnd);
        GSTestRun(TestUIHandleEvent);

        return NULL;
}

int main(int argC, char **argV) {
        char *result = RunAllTests();
        if (result != NULL) {
                printf("%s\n", result);
        } else {
                printf("ALL TESTS PASSED\n");
        }
        printf("opcode_test tests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
