/*
  Chip-8 CPU State
*/
#include <string.h> // memset
#include <stdlib.h> // malloc, free
#include <stdio.h>

#include "system.h"

#define GRAPHICS_WIDTH 64
#define GRAPHICS_HEIGHT 32
#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define GRAPHICS_MEM_SIZE GRAPHICS_WIDTH*GRAPHICS_HEIGHT
#define STACK_SIZE 16
#define NUM_KEYS 16
#define FONT_SIZE 80

typedef void *(*allocator)(size_t);
typedef void (*deallocator)(void *);

static unsigned char MEMORY[MEMORY_SIZE];
static unsigned char GFX[GRAPHICS_MEM_SIZE];
static allocator ALLOCATOR = malloc;
static deallocator DEALLOCATOR = free;

static unsigned char fontset[FONT_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void SystemDebug(struct system *s) {
        printf("struct system {\n");
        printf("\tdelayTimer__ %d\n", s->delayTimer);
        printf("\tfontp:______ 0x%04X\n", s->fontp);
        printf("\tgfx:________ %p\n", s->gfx);
        printf("\ti:__________ 0x%04X\n", s->i);
        printf("\tmemory:_____ %p\n", s->memory);
        printf("\tpc:_________ 0x%04X\n", s->pc);
        printf("\tsoundTimer:_ %d\n", s->soundTimer);
        printf("\tsp:_________ %d\n", s->sp);
        printf("\n");

        for (int i=0; i<8; i++) {
                printf("\tv[%X]:____ 0x%04X\tv[%X]:____ 0x%04X\n", i, s->v[i], i+8, s->v[i+8]);
        }
        printf("\n");

        for (int i=0; i<8; i++) {
                printf("\tstack[%X]: 0x%04X\tstack[%X]: 0x%04X\n", i, s->stack[i], i+8, s->stack[i+8]);
        }
        printf("\n");

        for (int i=0; i<8; i++) {
                char bool1 = 'F';
                if (s->key[i]) {
                        bool1 = 'T';
                }

                char bool2 = 'F';
                if (s->key[i+8]) {
                        bool2 = 'T';
                }

                printf("\tkey[0x%X]:__ %c\tkey[0x%X]:__ %c\n", i, bool1, i+8, bool2);
        }

        printf("}\n");
}

void SystemMemControl(allocator Alloc, deallocator Dealloc) {
        ALLOCATOR = Alloc;
        DEALLOCATOR = Dealloc;
}

struct system *SystemInit() {
        struct system *s = (struct system *)malloc(sizeof(struct system));
        memset(s, 0, sizeof(struct system));

        s->memory = MEMORY;
        memset(MEMORY, 0, MEMORY_SIZE);

        s->gfx = GFX;
        memset(GFX, 0, GRAPHICS_MEM_SIZE);

        s->pc = 0x200;

        s->fontp = 0;
        for (int i=s->fontp; i<FONT_SIZE; i++) {
                s->memory[i] = fontset[i];
        }

        s->waitForKey = -1;

        s->displayWidth = 64;
        s->displayHeight = 32;

        return s;
}

void SystemFree(struct system *s) {
        DEALLOCATOR(s);
}

// Each opcode is a two-byte instruction, so we have to double increment each time.
void SystemIncrementPC(struct system *s) {
        s->pc += 2;
}

unsigned short SystemFontSprite(struct system *s, unsigned int index) {
        return s->fontp + (index * 5);
}

int SystemLoadProgram(struct system *s, unsigned char *m, unsigned int size) {
        unsigned char *mem = &s->memory[0x200];
        unsigned short max_size = MEMORY_SIZE - 0x200;

        if (size > max_size) {
                return 0;
        }

        for (int i=0; i<size; i++) {
                mem[i] = m[i];
        }

        return !0;
}

void SystemDecrementTimers(struct system *s) {
        if (s->delayTimer > 0) {
                s->delayTimer--;
        }

        if (s->soundTimer > 0) {
                s->soundTimer--;
        }
}

void SystemPushStack(struct system *s) {
        if (s->sp > 0xF) {
                return;
                // TODO: ERROR?!?
        }

        s->stack[s->sp] = s->pc;
        s->sp++;
}

void SystemPopStack(struct system *s) {
        if (s->sp < 1) {
                return;
                // TODO: ERROR?!?
        }

        s->stack[s->sp] = 0; // Debug - set to zero after "free."
        s->sp--;
        s->pc = s->stack[s->sp];
}

void SystemClearScreen(struct system *s) {
        memset(s->gfx, 0, GRAPHICS_MEM_SIZE);
}

// Display: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
// and a height of N pixels. Each row of 8 pixels is read as bit-coded starting
// from memory location I; I value doesn’t change after the execution of this
// instruction. As described above, VF is set to 1 if any screen pixels are
// flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t
// happen.
// I'm assuming (VX, VY) is the lower-left corner of the sprint, not the center.
void SystemDrawSprite(struct system *s, unsigned int x_pos, unsigned int y_pos, unsigned int height) {
        s->v[15] = 0;

        for (int y = 0; y < height; y++) {
                // I contains a 1-byte bitmap representing a line of the sprite.
                // [XXXX XXXX]
                unsigned char pixel = s->memory[s->i + y];

                for (int x = 0; x < 8; x++) {
                        if ((pixel & (0x80 >> x)) == 0) { // This line taken from www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
                                continue;
                        }

                        int y_off = (y_pos + y) * GRAPHICS_WIDTH;
                        int x_off = (x_pos + x);
                        int pos = y_off + x_off;

                        if (s->gfx[pos] == 0xFF) {
                                s->v[15] = 1;
                        }

                        s->gfx[pos] ^= 0xFF;
                }
        }
}

void SystemClearKeys(struct system *s) {
        memset(s->key, 0, 16);
}
