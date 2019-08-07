/******************************************************************************
  File: sound_test.c
  Created: 2019-08-04
  Updated: 2019-08-06
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <dlfcn.h> // dlsym, RTLD_NEXT
#include <stdio.h>

#include "gstest.h"

// Overwrite soundio functions with testing versions.
#define soundio_outstream_pause(x,y) SoundioOutstreamPause(x,y)
#define soundio_outstream_start(x) SoundioOutstreamStart(x)

#include "../sound.h"
#include "../sound.c"

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
        void (*libcFree)(void *) = NULL;
        *(void **)&libcFree = dlsym(RTLD_NEXT, "free");
        if (useCustomFree) {
                customFreeCount++;
        }
        libcFree(p);
}

int soundStartCount = 0;
int soundPauseCount = 0;
int soundUnpauseCount = 0;

int SoundioOutstreamStart(struct SoundIoOutStream *ignore) {
        soundStartCount++;
        return 0;
}

int SoundioOutstreamPause(struct SoundIoOutStream *ignore, bool pause) {
        if (pause) {
                soundPauseCount++;
        } else {
                soundUnpauseCount++;
        }

        return 0;
}

//------------------------------------------------------------------------------
// Tests
//------------------------------------------------------------------------------

static char *TestSoundInit() {
        int before = soundStartCount;
        struct sound *sound = SoundInit();

        GSTestAssert(soundStartCount == before + 1, "got %d, want %d", soundStartCount, before + 1);
        GSTestAssert(sound->lib != NULL, "got %d, didn't want %d", sound->lib, NULL);
        GSTestAssert(sound->dev != NULL, "got %d, didn't want %d", sound->dev, NULL);
        GSTestAssert(sound->stream != NULL, "got %d, didn't want %d", sound->stream, NULL);
        GSTestAssert(sound->stream->format == SoundIoFormatFloat32NE, "got %d, want %d", sound->stream->format, SoundIoFormatFloat32NE);
        GSTestAssert(sound->stream->write_callback == WriteCallback, "got %p, want %p", sound->stream->write_callback, WriteCallback);

        SoundDeinit(sound);

        return NULL;
}

static char *TestSoundDeinit() {
        struct sound *sound = SoundInit();

        int before = customFreeCount;
        useCustomFree = 1;
        SoundDeinit(sound);
        useCustomFree = 0;
        GSTestAssert(customFreeCount > before, "got %d, want greater than %d", customFreeCount, before);

        return NULL;
}

static char *TestSoundPlay() {
        struct sound *sound = SoundInit();

        int before = soundUnpauseCount;
        SoundPlay(sound);
        GSTestAssert(soundUnpauseCount == before + 1, "got %d, want %d", soundUnpauseCount, before + 1);

        SoundDeinit(sound);

        return NULL;
}

static char *TestSoundStop() {
        struct sound *sound = SoundInit();

        int before = soundPauseCount;
        SoundStop(sound);
        GSTestAssert(soundPauseCount == before + 1, "got %d, want %d", soundPauseCount, before + 1);

        SoundDeinit(sound);

        return NULL;
}


static char *RunAllTests() {
        GSTestRun(TestSoundInit);
        GSTestRun(TestSoundDeinit);
        GSTestRun(TestSoundPlay);
        GSTestRun(TestSoundStop);
        return NULL;
}

int main(int argC, char **argV) {
        char *result = RunAllTests();
        if (result != NULL) {
                printf("%s\n", result);
        } else {
                printf("ALL TESTS PASSED\n");
        }
        printf("opcode_test tests run: %d\n", GSTestNumTestsRun);

        return result != NULL;
}
