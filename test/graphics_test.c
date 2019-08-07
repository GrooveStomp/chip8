/******************************************************************************
  File: graphics_test.c
  Created: 2019-07-23
  Updated: 2019-08-06
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <dlfcn.h> // dlsym, RTLD_NEXT
#include <stdio.h>

#include "gstest.h"

#include "../graphics.h"
#include "../graphics.c"

// Disables warnings on some sloppy string formatting usage. ie., function
// pointer instead of void pointer.
#pragma GCC diagnostic ignored "-Wformat="

int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

//------------------------------------------------------------------------------
// Helper functions and globals
//------------------------------------------------------------------------------

int customFreeCount = 0;
int useCustomFree = 0;

void free(void *p) {
        static void (*libcFree)(void *) = NULL;
        if (NULL == libcFree) {
                *(void **)&libcFree = dlsym(RTLD_NEXT, "free");
        }

        if (useCustomFree) {
                customFreeCount++;
        }

        libcFree(p);
}

//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------

static char *TestGraphicsInit() {
        { // With debugging enabled
                struct graphics *graphics = GraphicsInit(1);

                GSTestAssert(graphics->debug == 1, "got %d, want %d", graphics->debug, 1);
                GSTestAssert(graphics->displayWidth, "got %d, want %d", graphics->displayWidth, DISPLAY_WIDTH_WITH_DEBUGGER);
                GSTestAssert(graphics->displayHeight, "got %d, want %d", graphics->displayHeight, DISPLAY_HEIGHT_WITH_DEBUGGER);
                GSTestAssert(graphics->sdlWindow != NULL, "got %d, didn't want %d", graphics->sdlWindow, NULL);
                GSTestAssert(graphics->glContext != NULL, "got %d, didn't want %d", graphics->glContext, NULL);
                GSTestAssert(graphics->textureData != NULL, "got %d, didn't want %d", graphics->textureData, NULL);

                GraphicsDeinit(graphics);
        }

        { // Without debugging enabled
                struct graphics *graphics = GraphicsInit(0);

                GSTestAssert(graphics->debug == 0, "got %d, want %d", graphics->debug, 0);
                GSTestAssert(graphics->displayWidth, "got %d, want %d", graphics->displayWidth, CHIP8_DISPLAY_WIDTH * DISPLAY_SCALE);
                GSTestAssert(graphics->displayHeight, "got %d, want %d", graphics->displayHeight, CHIP8_DISPLAY_HEIGHT * DISPLAY_SCALE);
                GSTestAssert(graphics->sdlWindow != NULL, "got %d, didn't want %d", graphics->sdlWindow, NULL);
                GSTestAssert(graphics->glContext != NULL, "got %d, didn't want %d", graphics->glContext, NULL);
                GSTestAssert(graphics->textureData != NULL, "got %d, didn't want %d", graphics->textureData, NULL);

                GraphicsDeinit(graphics);
        }

        return NULL;
}

static char *TestGraphicsDeinit() {
        struct graphics *graphics = GraphicsInit(0);

        int before = customFreeCount;
        useCustomFree = 1;
        GraphicsDeinit(graphics);
        useCustomFree = 0;
        GSTestAssert(customFreeCount > before, "got %d, want greater than %d", customFreeCount, before);

        return NULL;
}

static char *TestGraphicsSDLWindow() {
        struct graphics *graphics = GraphicsInit(1);

        SDL_Window *result = GraphicsSDLWindow(graphics);
        GSTestAssert(result == graphics->sdlWindow, "got %p, want %p", result, graphics->sdlWindow);

        GraphicsDeinit(graphics);

        return NULL;
}

static char *RunAllTests() {
        GSTestRun(TestGraphicsInit);
        GSTestRun(TestGraphicsDeinit);
        GSTestRun(TestGraphicsSDLWindow);
        return NULL;
}

int main(int argC, char **argV) {
        printf("graphics_test:\n");
        char *result = RunAllTests();
        if (result != NULL) {
                printf("\t%s\n", result);
        } else {
                printf("\tALL TESTS PASSED\n");
        }
        printf("\ttests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
