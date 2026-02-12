#include "Thread.h"

#include <cstdio>
#include <sstream>
#include <sys/syscall.h>
#include <sys/types.h>

/**
 * @brief 获取当前线程的tid
 * @return 线程ID
 */
pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}


// 静态成员变量初始化
std::atomic<int32_t> Thread::m_numCreated(0);

Thread::Thread(ThreadFunc func, const std::string& name)
    : m_started(false),
      m_joined(false),
      m_thread(nullptr),
      m_tid(0),
      m_func(std::move(func)),
      m_name(name) {
    setDefaultName();
    ++m_numCreated;
}

Thread::~Thread() {
    if (m_started && !m_joined) {
        // 如果线程已启动但未join，则detach线程
        m_thread->detach();
    }
}

void Thread::start() {
    m_started = true;
    m_thread = std::make_shared<std::thread>([this]() {
        m_tid = gettid();
        m_func();
    });
}

void Thread::join() {
    if (m_started && !m_joined) {
        m_thread->join();
        m_joined = true;
    }
}

void Thread::setDefaultName() {
    if (m_name.empty()) {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", m_numCreated.load());
        m_name = buf;
    }
}
