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

        static const double msPerFrame = HZ_TO_MS(60);

        while (!ThreadSyncShouldShutdown(ctx->threadSync)) {
                if (SystemDebugIsEnabled(ctx->sys)) continue;

                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                SystemDecrementTimers(ctx->sys);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsed_time = S_TO_MS(end.tv_sec - start.tv_sec);
                elapsed_time += NS_TO_MS(end.tv_nsec - start.tv_nsec);

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = MS_TO_NS(msPerFrame - elapsed_time) };
                nanosleep(&sleep, NULL);
        }

        return NULL;
}
