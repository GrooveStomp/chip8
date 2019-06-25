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
