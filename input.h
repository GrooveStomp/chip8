/******************************************************************************
  File: input.h
  Date: 2019-07-07
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
          by Aaron Oman (See LICENSE)
 ******************************************************************************/
#ifndef INPUT_VERSION
#define INPUT_VERSION "0.1.0"

#include "SDL.h"

struct input;
struct system;

struct input *
InputInit();

int
InputCheck(struct input *i, struct system *s, SDL_Event *e);

#endif // INPUT_VERSION
