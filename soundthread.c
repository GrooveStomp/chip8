/******************************************************************************
  File: soundthread.c
  Created: 2019-07-25
  Updated: 2019-07-31
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/

//! \file soundthread.c

//! \brief Thread for sound playback
//!
//! Sound playback is triggered via the CHIP-8's sound timer.
//! When the timer reaches zero, a sound is played back.
//! The specifications seem loose on what this means exactly, so this emulator
//! plays back a tone of 440hz for 200ms.
//!
//! \param[in] context struct thread_args casted to void*
//! \return NULL
void *SoundThread(void *context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpointer-arith"
        struct thread_args *ctx = (struct thread_args *)context;
        #pragma GCC diagnostic pop

        struct sound *sound = SoundInit();
        if (NULL == sound) {
                fprintf(stderr, "Couldn't initialize sound");
                return NULL;
        }

        struct timer *timer = TimerInit(200);
        int playing = 0;

        while (!ThreadSyncShouldShutdown(ctx->threadSync)) {
                if (SystemSoundTriggered(ctx->sys)) {
                        TimerReset(timer);
                        SystemSoundSetTrigger(ctx->sys, 0);
                        playing = 1;
                        SoundPlay(sound);
                }

                if (playing && TimerHasElapsed(timer)) {
                        SoundStop(sound);
                        playing = 0;
                }
        }

        SoundDeinit(sound);

        return NULL;
}
