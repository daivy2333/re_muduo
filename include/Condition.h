#pragma once

#include "Mutex.h"
#include <pthread.h>
#include <errno.h>

/**
 * @brief Condition类，封装了pthread_cond_t
 */
class Condition : noncopyable {
public:
    explicit Condition(MutexLock& mutex)
        : mutex_(mutex) {
        pthread_cond_init(&pcond_, nullptr);
    }

    ~Condition() {
        pthread_cond_destroy(&pcond_);
    }

    void wait() {
        pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
    }

    void waitForSeconds(double seconds) {
        struct timespec timespec;
        clock_gettime(CLOCK_REALTIME, &timespec);

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

        timespec.tv_sec += static_cast<time_t>((timespec.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
        timespec.tv_nsec = static_cast<long>((timespec.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

        pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &timespec);
    }

    void notify() {
        pthread_cond_signal(&pcond_);
    }

    void notifyAll() {
        pthread_cond_broadcast(&pcond_);
    }

private:
    MutexLock& mutex_;
    pthread_cond_t pcond_;
};
