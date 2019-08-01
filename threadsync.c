/******************************************************************************
  File: threadsync.c
  Created: 2019-07-25
  Updated: 2019-08-01
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
//! \file threadsync.c

//! Cross-thread signalling
struct thread_sync {
        pthread_rwlock_t lock;
        int shouldShutdown; //!< Signal to trigger graceful thread shutdown
};

//! \brief Creates and initializes a new thread_sync object instance
//! \return The initialized thread_sync object
struct thread_sync *ThreadSyncInit() {
        struct thread_sync *s = (struct thread_sync *)malloc(sizeof(struct thread_sync));
        s->shouldShutdown = 0;

        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, 1);

        if (0 != pthread_rwlock_init(&s->lock, &attr)) {
                fprintf(stderr, "Couldn't initialize thread_sync rwlock");
                return NULL;
        }

        return s;
}

//! \brief De-initializes and frees memory for the given thread_sync object
//! \param[in,out] threadSync The initialized thread_sync object to be cleaned and reclaimed
void ThreadSyncDeinit(struct thread_sync *threadSync) {
        if (NULL == threadSync) return;

        if (0 != pthread_rwlock_destroy(&threadSync->lock)) {
                fprintf(stderr, "Couldn't destroy thread_Sync lock");
        }

        free(threadSync);
}

//! \brief atomically updates shouldShutdown status of thread_sync state
//! \param[in,out] threadSync thread_sync state to read
//! \return 1 when threadSync has been triggered, otherwise 0
int ThreadSyncShouldShutdown(struct thread_sync *threadSync) {
        if (0 != pthread_rwlock_rdlock(&threadSync->lock)) {
                fprintf(stderr, "Failed to lock thread_sync rwlock");
                return 0;
        }

        int result = threadSync->shouldShutdown;
        pthread_rwlock_unlock(&threadSync->lock);

        return result;
}

//! \brief atomically updates shouldShutdown status of thread_sync state
//! \param[in,outv threadSync thread_sync state to update
void ThreadSyncSignalShutdown(struct thread_sync *threadSync) {
        if (NULL == threadSync) return;

        if (0 != pthread_rwlock_wrlock(&threadSync->lock)) {
                fprintf(stderr, "Failed to lock thread_sync rwlock");
                return;
        }

        threadSync->shouldShutdown = 1;
        pthread_rwlock_unlock(&threadSync->lock);
}
