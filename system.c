/*
  Chip-8 CPU State
*/

#ifndef SYSTEM_C
#define SYSTEM_C

#include <string.h> // memset

struct system {
        // 35 opcodes
        unsigned short opcode;

        // 4k memory
        unsigned char memory[4096];

        // CPU registers: The Chip 8 has 15 8-bit general purpose registers
        // named V0,V1 up to VE. The 16th register is used for the ‘carry
        // flag’. Eight bits is one byte so we can use an unsigned char for this
        // purpose:
        unsigned char v[16];

        // There is an Index register I and a program counter (pc) which can
        // have a value from 0x000 to 0xFFF
        unsigned short i;
        unsigned short pc;

        // System memory map:
        // 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
        // 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
        // 0x200-0xFFF - Program ROM and work RAM

        // The graphics of the Chip 8 are black and white and the screen has a
        // total of 2048 pixels (64 x 32). This can easily be implemented using
        // an array that hold the pixel state (1 or 0):
        unsigned char gfx[64 * 32];

        // There are two timer registers that count at 60 Hz. When set above
        // zero they will count down to zero.
        unsigned char delay_timer;
        unsigned char sound_timer;

        unsigned short stack[16];
        unsigned short sp;

        // Finally, the Chip 8 has a HEX based keypad (0x0-0xF), you can use an
        // array to store the current state of the key.
        unsigned char key[16];
};

void SystemInit(struct system *s) {
        memset(s->memory, 0, 4096);
        memset(s->gfx, 0, 64 * 32);
        memset(s->key, 0, 16);
}

// Each opcode is a two-byte instruction, so we have to double increment each time.
void SystemIncrementPC(struct system *s) {
        s->pc += 2;
}

#endif // SYSTEM_C
