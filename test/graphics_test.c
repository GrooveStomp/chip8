/******************************************************************************
  File: graphics_test.c
  Created: 2019-07-23
  Updated: 2019-08-04
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdio.h>

#include "gstest.h"

#include "../graphics.h"
#include "../graphics.c"

// Disables warnings on some sloppy string formatting usage. ie., function
// pointer instead of void pointer.
#pragma GCC diagnostic ignored "-Wformat="

int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

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

static char *TestGraphicsSDLWindow() {
        struct graphics *graphics = GraphicsInit(1);

        SDL_Window *result = GraphicsSDLWindow(graphics);
        GSTestAssert(result == graphics->sdlWindow, "got %p, want %p", result, graphics->sdlWindow);

        GraphicsDeinit(graphics);

        return NULL;
}

static char *RunAllTests() {
        GSTestRun(TestGraphicsInit);
        GSTestRun(TestGraphicsSDLWindow);
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
