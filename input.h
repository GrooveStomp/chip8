#ifndef INPUT_VERSION
#define INPUT_VERSION "0.1.0"

struct input;
struct system;

struct input *
InputInit();

int
InputCheck(struct input *i, struct system *s);

#endif // INPUT_VERSION
