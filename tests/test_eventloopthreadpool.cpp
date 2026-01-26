#include "EventLoopThreadPool.h"
#include "Eventloop.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <atomic>
#include <thread>

// 测试基本的事件循环线程池创建和启动
void test_basic_creation() {
    std::cout << "=== Test Basic Creation ===" << std::endl;

    EventLoop baseLoop;
    EventLoopThreadPool pool(&baseLoop, "TestPool");

    assert(!pool.started());
    assert(pool.name() == "TestPool");

    pool.setThreadNUm(2);
    pool.start();

    assert(pool.started());

    // 运行一段时间后退出
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    baseLoop.quit();

    std::cout << "Basic creation test passed" << std::endl;
}

// 测试获取所有循环
void test_get_all_loops() {
    std::cout << "=== Test Get All Loops ===" << std::endl;

    EventLoop baseLoop;
    EventLoopThreadPool pool(&baseLoop, "TestPool");

    // 测试没有子线程的情况
    pool.setThreadNUm(0);
    pool.start();

    auto loops = pool.getAllLoops();
    assert(loops.size() == 1);
    assert(loops[0] == &baseLoop);

    baseLoop.quit();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Get all loops test passed" << std::endl;
}

// 测试轮询分配循环
void test_get_next_loop() {
    std::cout << "=== Test Get Next Loop ===" << std::endl;

    EventLoop baseLoop;
    EventLoopThreadPool pool(&baseLoop, "TestPool");

    // 测试没有子线程的情况
    pool.setThreadNUm(0);
    pool.start();

    EventLoop* loop1 = pool.getNextLoop();
    EventLoop* loop2 = pool.getNextLoop();

    assert(loop1 == &baseLoop);
    assert(loop2 == &baseLoop);

    baseLoop.quit();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 测试有子线程的情况
    // 在新线程中创建第二个 EventLoop，避免在同一个线程中创建多个 EventLoop
    std::thread t([&]() {
        EventLoop baseLoop2;
        EventLoopThreadPool pool2(&baseLoop2, "TestPool2");
        pool2.setThreadNUm(2);
        pool2.start();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto loops = pool2.getAllLoops();
        assert(loops.size() == 2); // 只有 2 个子线程循环，不包含 baseLoop

        // 测试轮询分配
        EventLoop* next1 = pool2.getNextLoop();
        EventLoop* next2 = pool2.getNextLoop();
        EventLoop* next3 = pool2.getNextLoop();
        EventLoop* next4 = pool2.getNextLoop();

        // 验证轮询顺序 - 只在子线程循环中轮询
        assert(next1 == loops[0]);
        assert(next2 == loops[1]);
        assert(next3 == loops[0]); // 循环回到第一个
        assert(next4 == loops[1]);

        baseLoop2.quit();
    });
    t.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Get next loop test passed" << std::endl;
}

// 测试初始化回调
void test_init_callback() {
    std::cout << "=== Test Init Callback ===" << std::endl;

    EventLoop baseLoop;
    EventLoopThreadPool pool(&baseLoop, "TestPool");

    std::atomic<int> callbackCount(0);
    std::atomic<int> baseLoopCallback(0);

    EventLoopThreadPool::ThreadInitCallback cb = [&callbackCount, &baseLoopCallback](EventLoop* loop) {
        callbackCount++;
        if (loop->isInLoopThread()) {
            baseLoopCallback++;
        }
        std::cout << "Init callback called for loop" << std::endl;
    };

    pool.setThreadNUm(2);
    pool.start(cb);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 应该有2个子线程的回调被调用
    assert(callbackCount == 2);

    baseLoop.quit();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Init callback test passed" << std::endl;
}

// 测试在多个循环中运行任务
void test_run_in_multiple_loops() {
    std::cout << "=== Test Run In Multiple Loops ===" << std::endl;

    EventLoop baseLoop;
    EventLoopThreadPool pool(&baseLoop, "TestPool");

    pool.setThreadNUm(3);
    pool.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::atomic<int> counter(0);
    auto loops = pool.getAllLoops();

    // 在每个循环中运行任务
    for (auto loop : loops) {
        loop->runInLoop([&counter]() {
            counter++;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(counter == 3); // 只有 3 个子线程循环

    baseLoop.quit();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Run in multiple loops test passed" << std::endl;
}

// 测试并发获取循环
void test_concurrent_get_loop() {
    std::cout << "=== Test Concurrent Get Loop ===" << std::endl;

    EventLoop baseLoop;
    EventLoopThreadPool pool(&baseLoop, "TestPool");

    pool.setThreadNUm(4);
    pool.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::atomic<int> counter(0);
    const int numTasks = 100;

    // 在 baseLoop_ 所在的线程中获取所有循环
    auto loops = pool.getAllLoops();
    assert(loops.size() == 4); // 4 个子线程循环

    // 创建多个线程并发运行任务
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&counter, numTasks, loops]() {
            for (int j = 0; j < numTasks; ++j) {
                // 轮询选择循环
                EventLoop* loop = loops[j % loops.size()];
                loop->runInLoop([&counter]() {
                    counter++;
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    assert(counter == numTasks * 4);

    baseLoop.quit();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Concurrent get loop test passed" << std::endl;
}

int main() {
    std::cout << "=== EventLoopThreadPool Tests ===" << std::endl;

    test_basic_creation();
    test_get_all_loops();
    test_get_next_loop();
    test_init_callback();
    test_run_in_multiple_loops();
    test_concurrent_get_loop();

    std::cout << "=== All EventLoopThreadPool Tests Passed ===" << std::endl;
    return 0;
}
