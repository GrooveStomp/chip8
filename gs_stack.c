#include <string.h> // memset
#include <stdio.h> // fprintf, FILE
#include <stdlib.h> // malloc, free

struct gs_stack {
        void **Items;
        unsigned int NextItem;
        unsigned int MaxItems;
        const char *Error;
};

typedef void *(*allocator)(size_t);
typedef void (*deallocator)(void *);

static const char *ERROR_NONE = "No error";
static const char *ERROR_NULL_POINTER = "Stack is pointing at NULL";
static const char *ERROR_NO_ITEMS = "Can't pop an item when the stack is empty.";
static const char *ERROR_FULL = "Can't push an item when the stack is full.";

static const char *BANNER = "--------------------------------------------------------------------------------";

static allocator ALLOCATOR = malloc;
static deallocator DEALLOCATOR = free;

void GSStackMemControl(allocator Alloc, deallocator Dealloc) {
        ALLOCATOR = Alloc;
        DEALLOCATOR = Dealloc;
}

size_t AllocSize(unsigned int max) {
        unsigned int NumItems = sizeof(void *) * max;
        return sizeof(struct gs_stack) + NumItems;
}

// allocator: Function that implements the malloc interface.
struct gs_stack *GSStackInit(unsigned int max) {
        struct gs_stack *s;

        size_t Size = AllocSize(max);
        s = (struct gs_stack *)ALLOCATOR(Size);
        if (s == NULL) {
                return NULL;
        }

        s->Items = (void **)(s + sizeof(struct gs_stack));
        memset(s->Items, 0, sizeof(void *) * max);

        s->MaxItems = max;
        s->NextItem = 0;

        return s;
}

int GSStackPush(struct gs_stack *s, void *Val) {
        if (s->Items == NULL) {
                s->Error = ERROR_NULL_POINTER;
                return 0;
        }

        if (s->NextItem >= s->MaxItems) {
                s->Error = ERROR_FULL;
                return 0;
        }

        s->Items[s->NextItem] = Val;
        s->NextItem++;
        return !0;
}

void *GSStackPop(struct gs_stack *s) {
        if (s->Items == NULL) {
                s->Error = ERROR_NULL_POINTER;
                return NULL;
        }

        if (s->NextItem == 0) {
                s->Error = ERROR_NO_ITEMS;
                return NULL;
        }

        s->NextItem--;
        return s->Items[s->NextItem];
}

const char *GSStackErr(struct gs_stack *s) {
        return s->Error;
}

// Report should interpret a void* to the appropriate type and display something
// useful about it.
// This function prints a newline after invoking Report.
//
void GSStackDebug(struct gs_stack *s, FILE *f, void (*Report)(FILE *f, void*)) {
        fprintf(f, "%s\n", BANNER);
        fprintf(f, "Stack:\n");
        fprintf(f, "\tMax items: %d\n", s->MaxItems);
        fprintf(f, "\tNum items: %d\n", s->NextItem);

        for (int i=0; i<s->NextItem; i++) {
                fprintf(f, "\t[%d]: ", i);
                Report(f, s->Items[i]);
                fprintf(f, "\n");
        }

        fprintf(f, "%s\n", BANNER);
}
