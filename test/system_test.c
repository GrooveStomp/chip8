/******************************************************************************
  File: system_test.c
  Created: 2019-07-07
  Updated: 2019-08-07
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <dlfcn.h> // dlsym, RTLD_NEXT
#include <stdio.h>

#include "gstest.h"

#include "../system.h"
#include "../system.c"

// Disables warnings on some sloppy string formatting usage. ie., function
// pointer instead of void pointer.
#pragma GCC diagnostic ignored "-Wformat="

int GSTestNumTestsRun = 0;
char GSTestErrMsg[GSTestErrMsgSize];

//------------------------------------------------------------------------------
// Helper functions and globals
//------------------------------------------------------------------------------

int customFreeCount = 0;
int useCustomFree = 0;

void free(void *p) {
        static void (*libcFree)(void *) = NULL;
        if (NULL == libcFree) {
                *(void **)&libcFree = dlsym(RTLD_NEXT, "free");
        }

        if (useCustomFree) {
                customFreeCount++;
        }

        libcFree(p);
}

//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------

static char *TestSystemInit() {
        {
                struct system *system = SystemInit(0);

                GSTestAssert(system->prv != NULL, "got %p, didn't want %p", system->prv, NULL);
                GSTestAssert(system->memory == MEMORY, "got %p, want %p", system->memory, MEMORY);
                GSTestAssert(system->gfx == GFX, "got %p, want %p", system->gfx, GFX);
                GSTestAssert(system->pc == 0x200, "got 0x%02x, want 0x%02x", system->pc, 0x200);
                GSTestAssert(system->fontp == 0, "got %d, want %d", system->fontp, 0);
                for (int i = 0; i < FONT_SIZE; i++) {
                        GSTestAssert(system->memory[i] == fontset[i], "got 0x%02x, want 0x%02x", system->memory[i], fontset[i]);
                }
                GSTestAssert(system->prv->debug.enabled == 0, "got %d, want %d", system->prv->debug.enabled, 0);
                GSTestAssert(system->prv->debug.fetchAndDecode == 1, "got %d, want %d", system->prv->debug.fetchAndDecode, 1);
                GSTestAssert(system->prv->debug.execute == 0, "got %d, want %d", system->prv->debug.execute, 0);

                SystemDeinit(system);
        }

        {
                struct system *system = SystemInit(1);
                GSTestAssert(system->prv->debug.enabled == 1, "got %d, want %d", system->prv->debug.enabled, 1);
                SystemDeinit(system);
        }

        return NULL;
}

static char *TestSystemDeinit() {
        struct system *system = SystemInit(0);

        int before = customFreeCount;
        useCustomFree = 1;
        SystemDeinit(system);
        useCustomFree = 0;
        GSTestAssert(customFreeCount > before, "got %d, want greater than %d", customFreeCount, before);

        return NULL;
}


static char *TestSystemIncrementPC() {
        struct system *system = SystemInit(0);
        int pc = system->pc;
        SystemIncrementPC(system);
        GSTestAssert(system->pc == pc + 2, "got %d, want %d", system->pc, pc + 2);
        SystemDeinit(system);

        return NULL;
}

static char *TestSystemFontSprite() {
        struct system *system = SystemInit(0);

        for (int i = 0; i < 0xF; i++) {
                int expectedPtr = system->fontp + (i * 5);
                GSTestAssert(SystemFontSprite(system, i) == expectedPtr, "got %p, want %p", SystemFontSprite(system, i), expectedPtr);
        }

        SystemDeinit(system);

        return NULL;
}

static char *TestSystemLoadProgram() {
        struct system *system = SystemInit(0);

        int result;
        unsigned char *rom;

        { // Rom is too big
                rom = malloc(MEMORY_SIZE);
                result = SystemLoadProgram(system, rom, MEMORY_SIZE);
                GSTestAssert(result == 0, "got %d, want %d", result, 0);
                free(rom);
        }

        { // Rom fits
                rom = malloc(MEMORY_SIZE / 2);
                result = SystemLoadProgram(system, rom, MEMORY_SIZE/2);
                GSTestAssert(result != 0, "got %d, didn't want %d", result, 0);
                for (int i = 0; i < MEMORY_SIZE / 2; i++) {
                        if (system->memory[0x200 + i] != rom[i]) {
                                GSTestAssert(0, "got %c, want %c", system->memory[0x200 + i], rom[i]);
                        }
                }
                free(rom);
        }

        SystemDeinit(system);

        return NULL;
}

static char *TestSystemStackPush() {
        struct system *system = SystemInit(0);

        { // Stack has room
                system->pc = 0x200;
                system->sp = 0;
                SystemStackPush(system);
                GSTestAssert(system->sp == 1, "got %d, want %d", system->sp, 1);
                GSTestAssert(system->stack[0] == 0x200, "got %d, want %d", system->stack[0], 0x200);
        }

        { // Stack has no room
                system->pc = 0x200;
                system->sp = 16;
                system->stack[0xF] = 0x111;
                SystemStackPush(system);
                GSTestAssert(system->sp == 16, "got %d, want %d", system->sp, 16);
                GSTestAssert(system->stack[0xF] == 0x111, "got %d, want %d", system->stack[0xF], 0x111);
        }

        SystemDeinit(system);

        return NULL;
}

static char *TestSystemStackPop() {
        struct system *system = SystemInit(0);

        { // Stack can be popped
                system->stack[0xF] = 0xF;
                system->stack[0xE] = 0xE;
                system->pc = 0x200;
                system->sp = 16;
                SystemStackPop(system);
                GSTestAssert(system->sp == 0xF, "got %d, want %d", system->sp, 0xF);
                GSTestAssert(system->pc == 0xF, "got 0x%02x, want 0x%02x", system->pc, 0xF);
        }

        { // Stack cannot be popped
                system->stack[0] = 1;
                system->pc = 0x200;
                system->sp = 0;
                SystemStackPop(system);
                GSTestAssert(system->sp == 0, "got %d, want %d", system->sp, 0);
                GSTestAssert(system->stack[0] == 1, "got %d, want %d", system->stack[0], 1);
                GSTestAssert(system->pc == 0x200, "got 0x%02x, want 0x%02x", system->pc, 0x200);
        }

        SystemDeinit(system);

        return NULL;
}

static char *TestSystemKey() {
       struct system *system = SystemInit(0);

       int got, want;

       for (int i = 0; i < 0xF; i++) {
               SystemKeySetPressed(system, i, 0);
       }

       want = 0;
       for (int i = 0; i < 0xF; i++) {
               got = SystemKeyIsPressed(system, i);
               GSTestAssert(got == want, "got %d, want %d", got, want);
       }

       SystemKeySetPressed(system, 3, 1);
       for (int i = 0; i < 0xF; i++) {
               want = 0;
               got = SystemKeyIsPressed(system, i);
               if (i == 3) want = 0xFF;

               GSTestAssert(got == want, "got %d, want %d", got, want);
       }

       want = 0xFF;
       for (int i = 0; i < 0xF; i++) {
               SystemKeySetPressed(system, i, want);
       }

       for (int i = 0; i < 0xF; i++) {
               got = SystemKeyIsPressed(system, i);
               GSTestAssert(got == want, "got %d, want %d", got, want);
       }

       SystemDeinit(system);

       return NULL;
}

static char *TestSystemQuit() {
        struct system *system = SystemInit(0);

        int got, want;

        // Default is not to be in a quit state.
        want = 0;
        got = SystemShouldQuit(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        // Signal quit, then test that status is correctly set.
        want = 1;
        SystemSignalQuit(system);
        got = SystemShouldQuit(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        SystemDeinit(system);

        return NULL;
}

static char *TestSystemTimers() {
        struct system *system = SystemInit(0);

        int got, want;

        // Verify setting -1 results in no change.
        SystemSetTimers(system, -1, -1);
        want = 0;
        got = SystemDelayTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 0;
        got = SystemSoundTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        // Verify we can set one timer at a time.
        SystemSetTimers(system, 5, -1);
        want = 5;
        got = SystemDelayTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 0;
        got = SystemSoundTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        // Verify decrement works only when value is above zero.
        SystemDecrementTimers(system);
        want = 4;
        got = SystemDelayTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 0;
        got = SystemSoundTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        // Verify we can set both timers;
        SystemSetTimers(system, 17, 25);
        want = 17;
        got = SystemDelayTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 25;
        got = SystemSoundTimer(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        SystemDeinit(system);

        return NULL;
}

static char *TestSystemWFK() {
        struct system *system = SystemInit(0);

        int got, want;

        // Initial state, not in WFK mode.
        want = 0;
        got = SystemWFKWaiting(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 0;
        got = SystemWFKChanged(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        // Signal WFK mode and verify state.
        SystemWFKSet(system, 5);
        want = 1;
        got = SystemWFKWaiting(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 0;
        got = SystemWFKChanged(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        // Signal WFK occurrence.
        SystemWFKOccurred(system, 6);
        want = 0;
        got = SystemWFKWaiting(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 1;
        got = SystemWFKChanged(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        // Verify register 5 receives hex key 6 as the pressed key.
        want = 6;
        got = system->v[5];
        GSTestAssert(got == want, "got %d, want %d", got, want);

        // Stop WFK mode.
        SystemWFKStop(system);
        want = 0;
        got = SystemWFKWaiting(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);
        want = 0;
        got = SystemWFKChanged(system);
        GSTestAssert(got == want, "got %d, want %d", got, want);

        SystemDeinit(system);

        return NULL;
}

static char *RunAllTests() {
        GSTestRun(TestSystemInit);
        GSTestRun(TestSystemDeinit);
        GSTestRun(TestSystemIncrementPC);
        GSTestRun(TestSystemFontSprite);
        GSTestRun(TestSystemLoadProgram);
        GSTestRun(TestSystemStackPush);
        GSTestRun(TestSystemStackPop);
        // GSTestRun(TestSystemGfxLock);
        // GSTestRun(TestSystemGfxUnlock);
        // GSTestRun(TestSystemClearScreen);
        // GSTestRun(TestSystemDrawSprite);
        GSTestRun(TestSystemWFK);
        GSTestRun(TestSystemTimers);
        // GSTestRun(TestSystemSoundTriggered);
        // GSTestRun(TestSystemSetTrigger);
        GSTestRun(TestSystemQuit);
        // GSTestRun(TestSystemDebugIsEnabled);
        // GSTestRun(TestSystemDebugSetEnabled);
        // GSTestRun(TestSystemDebugShouldFetchAndDecode);
        // GSTestRun(TestSystemDebugSetFetchAndExecute);
        // GSTestRun(TestSystemDebugShouldExecute);
        // GSTestRun(TestSystemDebugSetExecute);
        GSTestRun(TestSystemKey);
        return NULL;
}

int main(int argC, char **argV) {
        printf("system_test:\n");
        char *result = RunAllTests();
        if (result != NULL) {
                printf("\t%s\n", result);
        } else {
                printf("\tALL TESTS PASSED\n");
        }
        printf("\ttests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
