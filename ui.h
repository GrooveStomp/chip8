/******************************************************************************
  File: ui.h
  Date: 2019-07-07
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
          by Aaron Oman (See LICENSE)
 ******************************************************************************/
#ifndef UI_VERSION
#define UI_VERISON "0.1.0"

#include "SDL2/SDL.h"

struct system;
struct opcode;
struct ui;

struct ui_debug {
        int enabled;
        int resume;
        int waitForStep;
};

struct ui_debug *
UIDebugInfo(struct ui *);

void
UIMemControl(void *(*allocator)(size_t), void (*deallocator)(void *));

struct ui *
UIInit(int shouldBeEnabled, unsigned int widgetWidth, unsigned int widgetHeight, SDL_Window *window);

void
UIInputBegin(struct ui *u);

void
UIInputEnd(struct ui *u);

void
UIHandleEvent(struct ui *u, SDL_Event *event);

void
UIWidgets(struct ui *u, struct system *s, struct opcode *c);

void
UIRender(struct ui *u);

void
UIShutdown(struct ui *u);

#endif // UI_VERSION
