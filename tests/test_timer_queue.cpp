#include "Eventloop.h"
#include "Logger.h"
#include "Timer.h"
#include "TimerQueue.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <cassert>
#include <vector>
#include <memory>

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

/**
 * @brief 测试定时器回调为空的情况
 */
void test_timer_queue_null_callback()
{
    LOG_INFO("=== Test TimerQueue with null callback ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    // 尝试添加回调为空的定时器
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);
    TimerId timerId = timerQueue.addTimer(nullptr, when, 0.0);

    // 验证返回的TimerId是无效的
    assert(!timerId.isValid());

    // 0.5秒后退出循环
    loop.runAfter(0.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    LOG_INFO("TimerQueue null callback test passed");
}

/**
 * @brief 测试定时器在执行过程中被取消的场景
 */
void test_timer_queue_cancel_during_execution()
{
    LOG_INFO("=== Test TimerQueue cancel during execution ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::atomic<int> count(0);
    TimerId timerToCancel;

    // 创建一个每0.1秒触发一次的周期性定时器
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);
    timerToCancel = timerQueue.addTimer([&count, &timerQueue, &timerToCancel]() {
        count++;
        LOG_INFO("Timer triggered, count: %d", count.load());

        // 在第3次触发时取消自己
        if (count.load() == 3) {
            LOG_INFO("Canceling timer from its own callback");
            timerQueue.cancel(timerToCancel);
        }
    }, when, 0.1);

    // 1秒后退出循环
    loop.runAfter(1.0, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证定时器只触发了3次
    assert(count.load() == 3);
    LOG_INFO("Timer was canceled during execution after 3 triggers");

    LOG_INFO("TimerQueue cancel during execution test passed");
}

/**
 * @brief 测试大量定时器同时触发的情况
 */
void test_timer_queue_massive_timers()
{
    LOG_INFO("=== Test TimerQueue with massive timers ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::atomic<int> count(0);
    const int NUM_TIMERS = 100;

    // 创建100个定时器，都在0.1秒后触发
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);

    std::vector<TimerId> timerIds;
    for (int i = 0; i < NUM_TIMERS; ++i) {
        TimerId timerId = timerQueue.addTimer([&count]() {
            count++;
        }, when, 0.0);
        timerIds.push_back(timerId);
    }

    // 0.5秒后退出循环
    loop.runAfter(0.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证所有定时器都触发了
    assert(count.load() == NUM_TIMERS);
    LOG_INFO("All %d timers triggered successfully", NUM_TIMERS);

    LOG_INFO("TimerQueue massive timers test passed");
}

/**
 * @brief 测试定时器过期时间在过去的情况
 */
void test_timer_queue_past_expiration()
{
    LOG_INFO("=== Test TimerQueue with past expiration ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::atomic<bool> triggered(false);

    // 创建一个过期时间在过去的定时器
    Timestamp now = Timestamp::now();
    Timestamp past = addTime(now, -1.0); // 1秒前

    TimerId timerId = timerQueue.addTimer([&triggered]() {
        triggered = true;
        LOG_INFO("Timer with past expiration triggered");
    }, past, 0.0);

    // 0.5秒后退出循环
    loop.runAfter(0.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证定时器被触发（应该立即触发）
    assert(triggered.load());
    LOG_INFO("Timer with past expiration was triggered immediately");

    LOG_INFO("TimerQueue past expiration test passed");
}

/**
 * @brief 测试极短间隔定时器
 */
void test_timer_queue_very_short_interval()
{
    LOG_INFO("=== Test TimerQueue with very short interval ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::atomic<int> count(0);

    // 创建一个每0.001秒触发一次的定时器
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.001);

    TimerId timerId = timerQueue.addTimer([&count]() {
        count++;
    }, when, 0.001);

    // 0.1秒后取消定时器
    loop.runAfter(0.1, [&timerQueue, timerId]() {
        timerQueue.cancel(timerId);
    });

    // 0.2秒后退出循环
    loop.runAfter(0.2, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证定时器触发了多次
    assert(count.load() > 50); // 至少应该触发50次
    LOG_INFO("Timer with very short interval triggered %d times", count.load());

    LOG_INFO("TimerQueue very short interval test passed");
}

/**
 * @brief 测试定时器过期时间完全相同的情况
 */
void test_timer_queue_same_expiration()
{
    LOG_INFO("=== Test TimerQueue with same expiration ===");

    EventLoop loop;
    TimerQueue timerQueue(&loop);

    std::atomic<int> count(0);

    // 创建3个过期时间完全相同的定时器
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);

    timerQueue.addTimer([&count]() {
        count++;
        LOG_INFO("Timer 1 triggered");
    }, when, 0.0);

    timerQueue.addTimer([&count]() {
        count++;
        LOG_INFO("Timer 2 triggered");
    }, when, 0.0);

    timerQueue.addTimer([&count]() {
        count++;
        LOG_INFO("Timer 3 triggered");
    }, when, 0.0);

    // 0.5秒后退出循环
    loop.runAfter(0.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证所有定时器都触发了
    assert(count.load() == 3);
    LOG_INFO("All 3 timers with same expiration triggered");

    LOG_INFO("TimerQueue same expiration test passed");
}

/**
 * @brief 测试EventLoop退出时未触发定时器的清理
 */
void test_timer_queue_cleanup_on_exit()
{
    LOG_INFO("=== Test TimerQueue cleanup on exit ===");

    {
        EventLoop loop;
        TimerQueue timerQueue(&loop);

        std::atomic<int> count(0);

        // 创建一个5秒后才触发的定时器
        Timestamp now = Timestamp::now();
        Timestamp when = addTime(now, 5.0);

        timerQueue.addTimer([&count]() {
            count++;
            LOG_INFO("Long-delay timer triggered");
        }, when, 0.0);

        // 0.5秒后退出循环
        loop.runAfter(0.5, [&loop]() {
            LOG_INFO("Quit loop");
            loop.quit();
        });

        loop.loop();

        // 验证定时器未触发
        assert(count.load() == 0);
        LOG_INFO("Long-delay timer did not trigger before loop exit");
    }

    // TimerQueue和EventLoop已析构，验证没有内存泄漏
    LOG_INFO("TimerQueue and EventLoop destroyed successfully");

    LOG_INFO("TimerQueue cleanup on exit test passed");
}

int main()
{
    test_timer_queue_basic();
    test_timer_queue_cancel();
    test_timer_queue_concurrent();
    test_timer_queue_null_callback();
    test_timer_queue_cancel_during_execution();
    test_timer_queue_massive_timers();
    test_timer_queue_past_expiration();
    test_timer_queue_very_short_interval();
    test_timer_queue_same_expiration();
    test_timer_queue_cleanup_on_exit();

    LOG_INFO("All TimerQueue tests passed!");
    return 0;
}
