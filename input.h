/******************************************************************************
  File: input.h
  Created: 2019-07-21
  Updated: 2019-07-30
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#ifndef INPUT_VERSION
#define INPUT_VERSION "0.1.0"

#include "SDL.h"

struct input;
struct system;

struct input *
InputInit();

void
InputDeinit(struct input *);

void
InputCheck(struct input *i, struct system *s, SDL_Event *e);

#endif // INPUT_VERSION
