#pragma once

#include <pthread.h>
#include <cassert>
#include "CurrentThread.h"
#include "noncopyable.h"

/**
 * @brief MutexLock类，封装了pthread_mutex_t
 */
class MutexLock : noncopyable {
public:
    MutexLock() {
        pthread_mutex_init(&mutex_, nullptr);
    }

    ~MutexLock() {
        pthread_mutex_destroy(&mutex_);
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
    }

    void unlock() {
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }

    bool isLockedByThisThread() const {
        return holder_ == CurrentThread::tid();
    }

    pthread_mutex_t* getPthreadMutex() {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_ = 0;
};

/**
 * @brief MutexLockGuard类，RAII风格的互斥锁管理
 */
class MutexLockGuard : noncopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    MutexLock& mutex_;
};
