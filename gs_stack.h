#ifndef GS_STACK_VERSION
#define GS_STACK_VERSION "0.1.0"

#include <stdio.h> // FILE

struct gs_stack;

void
GSStackMemControl(void *(*allocator)(size_t), void (*deallocator)(void *));

struct gs_stack *
GSStackInit(unsigned int max);

int
GSStackPush(struct gs_stack *s, void *v);

void *
GSStackPop(struct gs_stack *s);

// If GSStackPush returns 0 or GSStackPop return NULL then this function
// will return an appropriate error message.
const char *
GSStackErr(struct gs_stack *s);

// Report should interpret a void* to the appropriate type and display something
// useful about it.
// This function prints a newline after invoking Report.
void
GSStackDebug(struct gs_stack *s, FILE *f, void (*report)(FILE *f, void *));

#endif // GS_STACK_VERSION
