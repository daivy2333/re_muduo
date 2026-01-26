#include "EventLoopThread.h"
#include "Eventloop.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <atomic>


// 测试基本的事件循环线程创建和启动
void test_basic_creation() {
    std::cout << "=== Test Basic Creation ===" << std::endl;

    EventLoopThread thread;
    EventLoop* loop = thread.startLoop();

    assert(loop != nullptr);

    // 在IO线程中检查 isInLoopThread
    std::atomic<bool> inLoopThread(false);
    loop->runInLoop([&]() {
        inLoopThread = loop->isInLoopThread();
    });

    // 等待检查完成
    while (!inLoopThread) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert(inLoopThread);

    // 运行一段时间后退出
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    loop->quit();

    std::cout << "Basic creation test passed" << std::endl;
}

// 测试线程命名
void test_thread_naming() {
    std::cout << "=== Test Thread Naming ===" << std::endl;

    EventLoopThread thread(EventLoopThread::ThreadInitCallback(), "MyEventLoopThread");
    EventLoop* loop = thread.startLoop();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    loop->quit();

    std::cout << "Thread naming test passed" << std::endl;
}

// 测试初始化回调
void test_init_callback() {
    std::cout << "=== Test Init Callback ===" << std::endl;

    std::atomic<bool> callbackCalled(false);
    EventLoop* callbackLoop = nullptr;

    EventLoopThread::ThreadInitCallback cb = [&callbackCalled, &callbackLoop](EventLoop* loop) {
        callbackCalled = true;
        callbackLoop = loop;
        std::cout << "Init callback called in thread" << std::endl;
    };

    EventLoopThread thread(cb);
    EventLoop* loop = thread.startLoop();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(callbackCalled);
    assert(callbackLoop == loop);

    loop->quit();

    std::cout << "Init callback test passed" << std::endl;
}

// 测试在事件循环中运行任务
void test_run_in_loop() {
    std::cout << "=== Test Run In Loop ===" << std::endl;

    std::atomic<int> counter(0);
    EventLoopThread thread;
    EventLoop* loop = thread.startLoop();

    // 在事件循环中运行任务
    loop->runInLoop([&counter]() {
        counter++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(counter == 1);

    loop->quit();

    std::cout << "Run in loop test passed" << std::endl;
}

// 测试多次创建和销毁
void test_multiple_creation() {
    std::cout << "=== Test Multiple Creation ===" << std::endl;

    for (int i = 0; i < 3; ++i) {
        EventLoopThread thread;
        EventLoop* loop = thread.startLoop();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        assert(loop != nullptr);
        loop->quit();
    }

    std::cout << "Multiple creation test passed" << std::endl;
}

// 测试多个并发的事件循环线程
void test_concurrent_loops() {
    std::cout << "=== Test Concurrent Loops ===" << std::endl;

    const int numThreads = 3;
    std::atomic<int> counter(0);
    std::vector<std::unique_ptr<EventLoopThread>> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(new EventLoopThread());
    }

    std::vector<EventLoop*> loops;
    for (auto& t : threads) {
        loops.push_back(t->startLoop());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 在每个循环中运行任务
    for (auto loop : loops) {
        loop->runInLoop([&counter]() {
            counter++;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(counter == numThreads);

    // 退出所有循环
    for (auto loop : loops) {
        loop->quit();
    }

    std::cout << "Concurrent loops test passed" << std::endl;
}

int main() {
    std::cout << "=== EventLoopThread Tests ===" << std::endl;

    test_basic_creation();
    test_thread_naming();
    test_init_callback();
    test_run_in_loop();
    test_multiple_creation();
    test_concurrent_loops();

    std::cout << "=== All EventLoopThread Tests Passed ===" << std::endl;
    return 0;
}
