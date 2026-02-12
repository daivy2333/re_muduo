#include "Eventloop.h"
#include "Timer.h"
#include "Timestamp.h"
#include "Thread.h"
#include "CurrentThread.h"
#include "Logger.h"

#include <iostream>
#include <unistd.h>

void timeout1() {
    LOG_INFO("Timeout 1!");
}

void timeout2() {
    LOG_INFO("Timeout 2!");
}

void timeout3() {
    LOG_INFO("Timeout 3!");
}

void test_eventloop_timer() {
    LOG_INFO("=== Test EventLoop timer ===");
    EventLoop loop;

    LOG_INFO("Main thread ID: %d", CurrentThread::tid());

    // 测试 runAfter
    loop.runAfter(1.0, timeout1);
    LOG_INFO("Execute timeout1 after 1 second");

    // 测试 runEvery
    auto timerId = loop.runEvery(2.0, timeout2);
    LOG_INFO("Execute timeout2 every 2 seconds");

    // 运行5秒后取消重复定时器
    loop.runAfter(5.0, [timerId, &loop](){
        LOG_INFO("Cancel periodic timer");
        loop.cancel(timerId);
    });

    // 在另一个线程中设置定时器
    Thread thread([&loop](){
        sleep(1);
        loop.runInLoop([](){
            LOG_INFO("Timer set in thread %d", CurrentThread::tid());
        });
    });
    thread.start();

    // 运行7秒
    loop.runAfter(7.0, [&loop](){
        LOG_INFO("Stop event loop after 7 seconds");
        loop.quit();
    });

    LOG_INFO("Start event loop");
    loop.loop();
    LOG_INFO("Event loop ended");

    thread.join();
}

void test_cross_thread_timer() {
    LOG_INFO("=== Test cross-thread timer ===");
    EventLoop loop;
    Thread timerThread([&loop](){
        // 在另一个线程中添加定时器
        LOG_INFO("Timer thread: %d", CurrentThread::tid());
        sleep(1);
        loop.runAfter(0.5, [](){
            LOG_INFO("Cross-thread timer triggered!");
        });
    });

    timerThread.start();
    loop.runAfter(3.0, [&loop](){ loop.quit(); });
    loop.loop();
    timerThread.join();
}

int main() {
    LOG_INFO("=== Timer and EventLoop integration test ===");

    LOG_INFO("1. Test basic timer functionality");
    test_eventloop_timer();

    LOG_INFO("2. Test cross-thread timer");
    test_cross_thread_timer();

    LOG_INFO("All integration tests passed!");
    return 0;
}
