/******************************************************************************
  File: timerthread.c
  Created: 2019-07-25
  Updated: 2019-07-31
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/

//! \file timerthread.c

//! \brief Thread for timer countdown
//!
//! The CHIP-8 has two timers: a delay timer and a sound timer.
//! Each timer decrements at a frequency of 60hz and then stays at zero once
//! zero is reached.
//!
//! \param[in] context struct thread_args casted to void*
//! \return NULL
void *timerTick(void *context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpointer-arith"
        struct thread_args *ctx = (struct thread_args *)context;
        #pragma GCC diagnostic pop

        static const double msPerFrame = HZ_TO_MS(60);

        while (!ThreadSyncShouldShutdown(ctx->threadSync)) {
                if (SystemDebugIsEnabled(ctx->sys)) continue;

                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                SystemDecrementTimers(ctx->sys);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsedTime = S_TO_MS(end.tv_sec - start.tv_sec);
                elapsedTime += NS_TO_MS(end.tv_nsec - start.tv_nsec);

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = MS_TO_NS(msPerFrame - elapsedTime) };
                nanosleep(&sleep, NULL);
        }

        return NULL;
}
