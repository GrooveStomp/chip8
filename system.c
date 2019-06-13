/*
  Chip-8 CPU State
*/
#include <string.h> // memset
#include <stdlib.h> // malloc

#include "system.h"
#include "gs_stack.h"

#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define GRAPHICS_MEM_SIZE 64 * 32
#define STACK_SIZE 16
#define NUM_KEYS 16
#define FONT_SIZE 80

static unsigned char MEMORY[MEMORY_SIZE];
static unsigned char GFX[GRAPHICS_MEM_SIZE];

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

void SystemInit(struct system *s) {
        memset(s, 0, sizeof(struct system));

        s->memory = MEMORY;
        memset(MEMORY, 0, MEMORY_SIZE);

        s->gfx = GFX;
        memset(GFX, 0, GRAPHICS_MEM_SIZE);

        s->pc = 0x200;

        s->subroutine_callers = GSStackInit(malloc, 12);

        s->fontp = 0;
        for (int i=s->fontp; i<FONT_SIZE; i++) {
                s->memory[i] = fontset[i];
        }
}

// Each opcode is a two-byte instruction, so we have to double increment each time.
void SystemIncrementPC(struct system *s) {
        s->pc += 2;
}

unsigned char *SystemFontSprite(struct system *s, unsigned int index) {
        return &s->memory[s->fontp + (index * 5)];
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
