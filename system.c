/******************************************************************************
  File: system.c
  Created: 2019-06-04
  Updated: 2019-07-23
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <string.h> // memset
#include <stdlib.h> // malloc, free
#include <stdio.h>
#include <pthread.h>

#include "system.h"

#define GRAPHICS_WIDTH 64
#define GRAPHICS_HEIGHT 32
#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define GRAPHICS_MEM_SIZE GRAPHICS_WIDTH*GRAPHICS_HEIGHT
#define STACK_SIZE 16
#define NUM_KEYS 16
#define FONT_SIZE 80

struct system_wfk { // wait for key
        unsigned char key; // 0 - 16
        int waiting;
        int justChanged;
        pthread_rwlock_t lock;
};

struct system_debug {
        int enabled;
        int fetchAndDecode;
        int execute;
        pthread_rwlock_t lock;
};

struct rect {
        unsigned int x1;
        unsigned int y1;
        unsigned int x2;
        unsigned int y2;
};

struct system_private {
        struct system_wfk wfk;
        struct system_debug debug;

        // There are two timer registers that count at 60 Hz. When set above
        // zero they will count down to zero.
        unsigned char delayTimer;
        unsigned char soundTimer;

        int soundTimerTriggered;

        int shouldQuit; // Inidicates if program is closed or otherwise quit.

        int isGfxDirty;
        struct rect gfxDirtyRegion;

        pthread_rwlock_t timerRwLock;
        pthread_rwlock_t soundRwLock;
        pthread_rwlock_t gfxRwLock;
        pthread_rwlock_t shouldQuitLock;
};

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

void SystemMemControl(allocator Alloc, deallocator Dealloc) {
        ALLOCATOR = Alloc;
        DEALLOCATOR = Dealloc;
}

struct system *SystemInit(int isDebugEnabled) {
        struct system_private *prv = (struct system_private *)ALLOCATOR(sizeof(struct system_private));
        memset(prv, 0, sizeof(struct system_private));

        struct system *s = (struct system *)ALLOCATOR(sizeof(struct system));
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

        s->prv = prv;

        s->prv->debug.enabled = isDebugEnabled;
        s->prv->debug.fetchAndDecode = 1;
        s->prv->debug.execute = 0;

        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, 1);

        if (0 != pthread_rwlock_init(&s->prv->debug.lock, &attr)) {
                fprintf(stderr, "Couldn't initialize system debugUi rwlock");
                return NULL;
        }

        if (0 != pthread_rwlock_init(&s->prv->shouldQuitLock, &attr)) {
                fprintf(stderr, "Couldn't initialize system shouldQuit rwlock");
                return NULL;
        }

        if (0 != pthread_rwlock_init(&s->prv->wfk.lock, &attr)) {
                fprintf(stderr, "Couldn't initialize system wfk rwlock");
                return NULL;
        }

        if (0 != pthread_rwlock_init(&s->prv->timerRwLock, &attr)) {
                fprintf(stderr, "Couldn't initialize system timer rwlock");
                return NULL;
        }

        if (0 != pthread_rwlock_init(&s->prv->soundRwLock, &attr)) {
                fprintf(stderr, "Couldn't initialize system sound rwlock");
                return NULL;
        }

        if (0 != pthread_rwlock_init(&s->prv->gfxRwLock, &attr)) {
                fprintf(stderr, "Couldn't initialize system gfx rwlock");
                return NULL;
        }

        return s;
}

void SystemDeinit(struct system *s) {
        if (NULL == s)
                return;

        if (0 != pthread_rwlock_destroy(&s->prv->debug.lock)) {
                fprintf(stderr, "Couldn't destroy system debugUi rwlock");
        }

        if (0 != pthread_rwlock_destroy(&s->prv->shouldQuitLock)) {
                fprintf(stderr, "Couldn't destroy system shouldQuit rwlock");
        }

        if (0 != pthread_rwlock_destroy(&s->prv->wfk.lock)) {
                fprintf(stderr, "Couldn't destroy system wfk rwlock");
        }

        if (0 != pthread_rwlock_destroy(&s->prv->timerRwLock)) {
                fprintf(stderr, "Couldn't destroy system timer rwlock");
        }

        if (0 != pthread_rwlock_destroy(&s->prv->soundRwLock)) {
                fprintf(stderr, "Couldn't destroy system sound rwlock");
        }

        if (0 != pthread_rwlock_destroy(&s->prv->gfxRwLock)) {
                fprintf(stderr, "Couldn't destroy system gfx rwlock");
        }

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

void SystemPushStack(struct system *s) {
        if (s->sp > 0xF) {
                return;
                fprintf(stderr, "SystemPushStack() Tried to push full stack\n");
        }

        s->stack[s->sp] = s->pc;
        s->sp++;
}

void SystemPopStack(struct system *s) {
        if (s->sp < 1) {
                return;
                fprintf(stderr, "SystemPopStack() Tried to pop empty stack\n");
        }

        s->stack[s->sp] = 0; // Unset "previous" stack head.
        s->sp--;
        s->pc = s->stack[s->sp];
}

int SystemGfxLock(struct system *s) {
        return pthread_rwlock_rdlock(&s->prv->gfxRwLock);
}

int SystemGfxUnlock(struct system *s) {
        return pthread_rwlock_unlock(&s->prv->gfxRwLock);
}

void SystemGfxPresent(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->gfxRwLock)) {
                fprintf(stderr, "Failed to lock system gfx rw lock");
                return;
        }

        s->prv->isGfxDirty = 0;
        pthread_rwlock_unlock(&s->prv->gfxRwLock);
}

void SystemClearScreen(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->gfxRwLock)) {
                fprintf(stderr, "Failed to lock system gfx rw lock");
                return;
        }

        s->prv->isGfxDirty = 1;
        s->prv->gfxDirtyRegion = (struct rect){ .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 };
        memset(s->gfx, 0, GRAPHICS_MEM_SIZE);
        pthread_rwlock_unlock(&s->prv->gfxRwLock);
}

// Display: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
// and a height of N pixels. Each row of 8 pixels is read as bit-coded starting
// from memory location I; I value doesn’t change after the execution of this
// instruction. As described above, VF is set to 1 if any screen pixels are
// flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t
// happen.
// I'm assuming (VX, VY) is the lower-left corner of the sprint, not the center.
void SystemDrawSprite(struct system *s, unsigned int x_pos, unsigned int y_pos, unsigned int height) {
        if (0 != pthread_rwlock_rdlock(&s->prv->gfxRwLock)) {
                fprintf(stderr, "Failed to lock system gfx rw lock");
                return;
        }

        s->prv->isGfxDirty = 1;
        s->prv->gfxDirtyRegion = (struct rect){ .x1 = x_pos, .y1 = y_pos, .x2 = x_pos + 8, .y2 = y_pos + height };
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
        pthread_rwlock_unlock(&s->prv->gfxRwLock);
}

void SystemWFKSet(struct system *s, unsigned char key) {
        if (0 != pthread_rwlock_wrlock(&s->prv->wfk.lock)) {
                fprintf(stderr, "Failed to lock system wfk rwlock");
                return;
        }
        s->prv->wfk.waiting = 1;
        s->prv->wfk.justChanged = 0;
        s->prv->wfk.key = key;
        pthread_rwlock_unlock(&s->prv->wfk.lock);
}

int SystemWFKWaiting(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->wfk.lock)) {
                fprintf(stderr, "Failed to lock system wfk rwlock");
                return 0;
        }
        int result = s->prv->wfk.waiting;
        pthread_rwlock_unlock(&s->prv->wfk.lock);

        return result;
}

void SystemWFKOccurred(struct system *s, unsigned char key) {
        if (0 != pthread_rwlock_wrlock(&s->prv->wfk.lock)) {
                fprintf(stderr, "Failed to lock system wfk rwlock");
                return;
        }
        s->prv->wfk.waiting = 0;
        s->prv->wfk.justChanged = 1;
        s->v[s->prv->wfk.key] = key;
        pthread_rwlock_unlock(&s->prv->wfk.lock);
}

int SystemWFKChanged(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->wfk.lock)) {
                fprintf(stderr, "Failed to lock system wfk rwlock");
                return 0;
        }
        int result = s->prv->wfk.justChanged;
        pthread_rwlock_unlock(&s->prv->wfk.lock);

        return result;
}

void SystemWFKStop(struct system *s) {
        if (0 != pthread_rwlock_wrlock(&s->prv->wfk.lock)) {
                fprintf(stderr, "Failed to lock system wfk rwlock");
                return;
        }
        s->prv->wfk.justChanged = 0;
        pthread_rwlock_unlock(&s->prv->wfk.lock);
}

void SystemDecrementTimers(struct system *s) {
        if (0 != pthread_rwlock_wrlock(&s->prv->timerRwLock)) {
                fprintf(stderr, "Failed to lock system timer rw lock");
                return;
        }

        if (s->prv->delayTimer > 0) {
                s->prv->delayTimer--;
        }

        if (s->prv->soundTimer > 0) {
                s->prv->soundTimer--;
        }

        pthread_rwlock_unlock(&s->prv->timerRwLock);
}

int SystemDelayTimer(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->timerRwLock)) {
                fprintf(stderr, "Failed to lock system timer rw lock");
                return -1;
        }

        int result = s->prv->delayTimer;
        pthread_rwlock_unlock(&s->prv->timerRwLock);

        return result;
}

int SystemSoundTimer(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->timerRwLock)) {
                fprintf(stderr, "Failed to lock system timer rw lock");
                return -1;
        }

        int result = s->prv->soundTimer;
        pthread_rwlock_unlock(&s->prv->timerRwLock);

        return result;
}

void SystemSetTimers(struct system *s, int dt, int st) {
        if (0 != pthread_rwlock_wrlock(&s->prv->timerRwLock)) {
                fprintf(stderr, "Failed to lock system timer rw lock");
                return;
        }

        if (dt != -1) {
                s->prv->delayTimer = dt;
        }
        if (st != -1) {
                s->prv->soundTimer = st;
        }
        pthread_rwlock_unlock(&s->prv->timerRwLock);
}

int SystemSoundTriggered(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->soundRwLock)) {
                fprintf(stderr, "Failed to lock system timer rw lock");
                return 0;
        }

        int result = (s->prv->soundTimerTriggered && s->prv->soundTimer == 0);
        pthread_rwlock_unlock(&s->prv->soundRwLock);

        return result;
}

void SystemSoundSetTrigger(struct system *s, int v) {
        if (0 != pthread_rwlock_wrlock(&s->prv->soundRwLock)) {
                fprintf(stderr, "Failed to lock system timer rw lock");
                return;
        }

        s->prv->soundTimerTriggered = v;
        pthread_rwlock_unlock(&s->prv->soundRwLock);
}

int SystemShouldQuit(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->shouldQuitLock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return 0;
        }

        int result = s->prv->shouldQuit;
        pthread_rwlock_unlock(&s->prv->shouldQuitLock);

        return result;
}

void SystemSignalQuit(struct system *s) {
        if (0 != pthread_rwlock_wrlock(&s->prv->shouldQuitLock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return;
        }

        s->prv->shouldQuit = 1;
        pthread_rwlock_unlock(&s->prv->shouldQuitLock);
}

int SystemDebugIsEnabled(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->debug.lock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return 0;
        }

        int result = s->prv->debug.enabled;
        pthread_rwlock_unlock(&s->prv->debug.lock);

        return result;
}

int SystemDebugShouldFetchAndDecode(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->debug.lock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return 0;
        }

        int result = s->prv->debug.fetchAndDecode;
        pthread_rwlock_unlock(&s->prv->debug.lock);

        return result;
}

int SystemDebugShouldExecute(struct system *s) {
        if (0 != pthread_rwlock_rdlock(&s->prv->debug.lock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return 0;
        }

        int result = s->prv->debug.execute;
        pthread_rwlock_unlock(&s->prv->debug.lock);

        return result;
}

void SystemDebugSetEnabled(struct system *s, int onOrOff) {
        if (0 != pthread_rwlock_wrlock(&s->prv->debug.lock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return;
        }

        s->prv->debug.enabled = onOrOff;
        pthread_rwlock_unlock(&s->prv->debug.lock);
}

void SystemDebugSetFetchAndDecode(struct system *s, int onOrOff) {
        if (0 != pthread_rwlock_wrlock(&s->prv->debug.lock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return;
        }

        s->prv->debug.fetchAndDecode = onOrOff;
        pthread_rwlock_unlock(&s->prv->debug.lock);
}

void SystemDebugSetExecute(struct system *s, int onOrOff) {
        if (0 != pthread_rwlock_wrlock(&s->prv->debug.lock)) {
                fprintf(stderr, "Failed to lock system shouldQuit rwlock");
                return;
        }

        s->prv->debug.execute = onOrOff;
        pthread_rwlock_unlock(&s->prv->debug.lock);
}
