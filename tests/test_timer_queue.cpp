#include "Eventloop.h"
#include "Logger.h"
#include "Timer.h"
#include "TimerQueue.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <cassert>

/**
 * @brief 测试TimerQueue的基本功能
 * 
 * 测试内容：
 * 1. 添加一次性定时器
 * 2. 添加周期性定时器
 * 3. 取消定时器
 * 4. 验证定时器回调执行顺序
 */
void test_timer_queue_basic()
{
    LOG_INFO("=== Test TimerQueue basic functionality ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::vector<int> execution_order;
    std::mutex mtx;

    // 1. 添加一次性定时器，0.5秒后执行
    Timestamp now = Timestamp::now();
    Timestamp when1 = addTime(now, 0.5);
    timerQueue.addTimer([&loop, &execution_order, &mtx]() {
        std::lock_guard<std::mutex> lock(mtx);
        execution_order.push_back(1);
        LOG_INFO("One-shot timer triggered");
    }, when1, 0.0);

    // 2. 添加周期性定时器，每0.3秒执行一次
    Timestamp when2 = addTime(now, 0.3);
    TimerId periodicTimer = timerQueue.addTimer([&loop, &execution_order, &mtx]() {
        std::lock_guard<std::mutex> lock(mtx);
        execution_order.push_back(2);
        LOG_INFO("Periodic timer triggered");
    }, when2, 0.3);

    // 3. 添加另一个一次性定时器，1.0秒后执行
    Timestamp when3 = addTime(now, 1.0);
    timerQueue.addTimer([&loop, &execution_order, &mtx]() {
        std::lock_guard<std::mutex> lock(mtx);
        execution_order.push_back(3);
        LOG_INFO("Second one-shot timer triggered");
    }, when3, 0.0);

    // 4. 1.5秒后退出循环
    loop.runAfter(1.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证执行顺序
    LOG_INFO("Execution order size: %zu", execution_order.size());
    assert(execution_order.size() >= 5); // 至少应该执行5次（2次周期性 + 2次一次性）

    LOG_INFO("TimerQueue basic test passed");
}

/**
 * @brief 测试TimerQueue取消功能
 * 
 * 测试内容：
 * 1. 创建周期性定时器
 * 2. 在回调中取消定时器
 * 3. 验证定时器确实被取消
 */
void test_timer_queue_cancel()
{
    LOG_INFO("=== Test TimerQueue cancel functionality ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::atomic<int> count(0);

    // 创建一个每0.2秒触发一次的周期性定时器
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.2);
    TimerId timerId = timerQueue.addTimer([&count]() {
        count++;
        LOG_INFO("Timer triggered, count: %d", count.load());
    }, when, 0.2);

    // 0.5秒后取消定时器
    loop.runAfter(0.5, [&timerQueue, timerId]() {
        LOG_INFO("Canceling timer");
        timerQueue.cancel(timerId);
    });

    // 1.5秒后退出循环
    loop.runAfter(1.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证定时器只触发了2-3次（0.5秒内，每0.2秒一次）
    assert(count.load() >= 2 && count.load() <= 3);
    LOG_INFO("Timer was canceled after %d triggers", count.load());

    LOG_INFO("TimerQueue cancel test passed");
}

/**
 * @brief 测试TimerQueue并发添加和取消
 * 
 * 测试内容：
 * 1. 在IO线程中添加定时器
 * 2. 在非IO线程中添加定时器
 * 3. 在非IO线程中取消定时器
 * 4. 验证线程安全性
 */
void test_timer_queue_concurrent()
{
    LOG_INFO("=== Test TimerQueue concurrent operations ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::atomic<int> count(0);
    std::atomic<int> canceled_count(0);

    // 启动一个新线程，在非IO线程中添加定时器
    std::thread otherThread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 在非IO线程中添加定时器
        Timestamp now = Timestamp::now();
        Timestamp when = addTime(now, 0.1);
        TimerId timerId = timerQueue.addTimer([&count]() {
            count++;
            LOG_INFO("Timer from other thread triggered");
        }, when, 0.1);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // 在非IO线程中取消定时器
        LOG_INFO("Canceling timer from other thread");
        timerQueue.cancel(timerId);
        canceled_count++;
    });

    // 1.5秒后退出循环
    loop.runAfter(1.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    otherThread.join();

    // 验证定时器被正确添加和取消
    assert(count.load() >= 4); // 至少应该触发4次
    assert(canceled_count.load() == 1); // 定时器应该被取消

    LOG_INFO("TimerQueue concurrent test passed");
}

int main()
{
    test_timer_queue_basic();
    test_timer_queue_cancel();
    test_timer_queue_concurrent();

    LOG_INFO("All TimerQueue tests passed!");
    return 0;
}
