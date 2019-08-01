/******************************************************************************
 * File: timer.c
 * Created: 2019-07-14
 * Updated: 2016-07-31
 * Creator: Aaron Oman
 * Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/

//! \file timer.c

#ifndef TIMER_VERSION
#define TIMER_VERSION "0.1.0"

#include <time.h> // nanosleep, struct timespec
#include <string.h> // memset
#include <stdio.h> // fprintf

//! \brief Queryable timer state used in soundthread.c
struct timer {
        struct timespec start;
        unsigned int waitMs;
};

//! \brief Returns a pointer to an initialized timer on the heap
//! \param[in] ms Time in ms after which the timer is considered to have "fired"
//! \return The initialized timer
struct timer *TimerInit(unsigned int ms) {
        struct timer *timer = (struct timer *)malloc(sizeof(struct timer));;

        clock_gettime(CLOCK_REALTIME, &timer->start);
        timer->waitMs = ms;

        return timer;
}

//! \brief Has this timer "fired"?
//! \param[in] timer Timer state to query
//! \return 1 if the timer has "fired" otherwise 0
int TimerHasElapsed(struct timer *timer) {
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);

        // Convert seconds to milliseconds.
        double elapsedTime = (now.tv_sec - timer->start.tv_sec) * 1000;
        // Convert nanoseconds to milliseconds.
        elapsedTime += ((now.tv_nsec - timer->start.tv_nsec) / 1000000);

        if (elapsedTime >= timer->waitMs) {
                return 1;
        }

        return 0;
}

//! \brief Reset the timer
//! \param[in,out] timer Timer state to reset
void TimerReset(struct timer *timer) {
        clock_gettime(CLOCK_REALTIME, &timer->start);
}

#endif // TIMER_VERSION
