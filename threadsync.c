/******************************************************************************
  File: threadsync.c
  Created: 2019-07-25
  Updated: 2019-07-25
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
struct thread_sync {
        pthread_rwlock_t lock;
        int shouldShutdown;
};

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

int ThreadSyncShouldShutdown(struct thread_sync *s) {
        if (0 != pthread_rwlock_rdlock(&s->lock)) {
                fprintf(stderr, "Failed to lock thread_sync rwlock");
                return 0;
        }

        int result = s->shouldShutdown;
        pthread_rwlock_unlock(&s->lock);

        return result;
}

void ThreadSyncSignalShutdown(struct thread_sync *s) {
        if (NULL == s) return;

        if (0 != pthread_rwlock_wrlock(&s->lock)) {
                fprintf(stderr, "Failed to lock thread_sync rwlock");
                return;
        }

        s->shouldShutdown = 1;
        pthread_rwlock_unlock(&s->lock);
}
