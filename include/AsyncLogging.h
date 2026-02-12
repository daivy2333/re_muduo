#pragma once

#include "LogStream.h"
#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"
#include "noncopyable.h"
#include <vector>
#include <memory>
#include <atomic>
#include <string>

class AsyncLogging : noncopyable {
public:
    AsyncLogging(const std::string& basename,
                 off_t rollSize,
                 int flushInterval = 3);

    ~AsyncLogging() { if (running_) stop(); }

    void append(const char* logline, int len);

    void start();
    void stop();

private:
    void threadFunc();

    typedef FixedBuffer Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int flushInterval_;
    std::atomic<bool> running_;
    const std::string basename_;
    const off_t rollSize_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};
