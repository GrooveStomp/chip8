/******************************************************************************
  File: timerthread.c
  Created: 2019-07-25
  Updated: 2019-07-27
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
void *timerTick(void *context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpointer-arith"
        struct thread_args *ctx = (struct thread_args *)context;
        #pragma GCC diagnostic pop

        static const double frequency = (1 / 60) * 1000; // 60 FPS in MS per frame.

        while (!ThreadSyncShouldShutdown(ctx->threadSync)) {
                if (SystemDebugIsEnabled(ctx->sys)) continue;

                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                SystemDecrementTimers(ctx->sys);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
                elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000000.0; // us to ms

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = (frequency - elapsed_time) * 1000000 };
                nanosleep(&sleep, NULL);
        }

        return NULL;
}
