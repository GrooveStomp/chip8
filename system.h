/******************************************************************************
  File: system.h
  Created: (No later than 2019-07-07)
  Updated: 2019-07-14
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#ifndef SYSTEM_VERSION
#define SYSTEM_VERSION "0.1.0"

struct system_private;

struct system {
        // 4k memory
        unsigned char *memory;

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
        unsigned char *gfx;

        unsigned short stack[16];
        unsigned short sp;

        // Finally, the Chip 8 has a HEX based keypad (0x0-0xF), you can use an
        // array to store the current state of the key.
        unsigned char key[16];

        unsigned short fontp;

        int waitForKey;
        unsigned int displayWidth;
        unsigned int displayHeight;

        struct system_private *prv;
};

void
SystemMemControl(void *(*allocator)(size_t), void (*deallocator)(void *));

struct system *
SystemInit();

void
SystemFree(struct system *s);

void
SystemIncrementPC(struct system *s);

unsigned short
SystemFontSprite(struct system *s, unsigned int index);

// Returns 0 if program could no be loaded, or non-zero otherwise.
int
SystemLoadProgram(struct system *s, unsigned char *m, unsigned int size);

void
SystemDecrementTimers(struct system *s);

void
SystemPushStack(struct system *s);

void
SystemPopStack(struct system *s);

void
SystemClearScreen(struct system *s);

void
SystemDrawSprite(struct system *s, unsigned int x, unsigned int y, unsigned int height);

void
SystemWFKSet(struct system *s, unsigned char key);

int
SystemWFKWaiting(struct system *s);

void
SystemWFKOccurred(struct system *s, unsigned char key);

int
SystemWFKChanged(struct system *s);

void
SystemWFKStop(struct system *s);

void
SystemDecrementTimers(struct system *s);

int
SystemDelayTimer(struct system *s);

int
SystemSoundTimer(struct system *s);

void
SystemSetTimers(struct system *s, int dt, int st);

int
SystemSoundTriggered(struct system *s);

void
SystemSoundSetTrigger(struct system *s, int v);

#endif // SYSTEM_VERSION
