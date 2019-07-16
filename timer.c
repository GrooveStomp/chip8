/******************************************************************************
 * File: timer.c
 * Created: 2019-07-14
 * Updated: 2016-07-16
 * Creator: Aaron Oman
 * Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#ifndef TIMER_VERSION
#define TIMER_VERSION "0.1.0"

#include <time.h> // nanosleep, struct timespec
#include <string.h> // memset
#include <stdio.h> // fprintf

struct timer {
        struct timespec start;
        unsigned int waitMs;
};

struct timer *TimerInit(unsigned int ms) {
        struct timer *timer = (struct timer *)malloc(sizeof(struct timer));;

        clock_gettime(CLOCK_REALTIME, &timer->start);
        timer->waitMs = ms;

        return timer;
}

void TimerDebug(struct timer *t, FILE *stream, char *name) {
        fprintf(stream, "struct timer %s {\n", name);
        fprintf(stream, "\tstruct timespec {\n");
        fprintf(stream, "\t\ttime_t tv_sec = %ld;\n", t->start.tv_sec);
        fprintf(stream, "\t\tlong   tv_nsec = %ld;\n", t->start.tv_nsec);
        fprintf(stream, "\t};\n");
        fprintf(stream, "\tunsigned int waitMs = %u;\n", t->waitMs);
        fprintf(stream, "};\n");
}

int TimerHasElapsed(struct timer *t) {
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);

        // Convert seconds to milliseconds.
        double elapsedTime = (now.tv_sec - t->start.tv_sec) * 1000;
        // Convert nanoseconds to milliseconds.
        elapsedTime += ((now.tv_nsec - t->start.tv_nsec) / 1000000);

        if (elapsedTime >= t->waitMs) {
                return 1;
        }

        return 0;
}

void TimerReset(struct timer *t) {
        clock_gettime(CLOCK_REALTIME, &t->start);
}

#endif // TIMER_VERSION
