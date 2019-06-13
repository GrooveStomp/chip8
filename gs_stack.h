#ifndef GS_STACK_H
#define GS_STACK_H

#include <stdio.h> // FILE

struct gs_stack;

struct gs_stack *GSStackInit(void *(*allocator)(size_t), unsigned int max);

int GSStackPush(struct gs_stack *s, void *v);

void *GSStackPop(struct gs_stack *s);

// If GSStackPush returns 0 or GSStackPop return NULL then this function
// will return an appropriate error message.
const char *GSStackErr(struct gs_stack *s);

// Report should interpret a void* to the appropriate type and display something
// useful about it.
// This function prints a newline after invoking Report.
void GSStackDebug(struct gs_stack *s, FILE *f, void (*report)(FILE *f, void *));

#endif // GS_STACK_H
