/******************************************************************************
  File: graphics.h
  Created: 2019-07-16
  Updated: 2019-07-30
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#ifndef GRAPHICS_VERSION
#define GRAPHICS_VERSION "0.1.0"

#include "SDL2/SDL.h"

struct system;
struct ui;

struct graphics *
GraphicsInit(int debug);

void
GraphicsDeinit(struct graphics *g);

SDL_Window *
GraphicsSDLWindow(struct graphics *g);

void
GraphicsPresent(struct graphics *g, struct system *s, void (*ui_render_fn)());

#endif // GRAPHICS_VERSION
