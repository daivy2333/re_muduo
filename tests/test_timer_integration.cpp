#include "Eventloop.h"
#include "Timer.h"
#include "Timestamp.h"
#include "Thread.h"
#include "CurrentThread.h"
#include "Logger.h"

#include <iostream>
#include <unistd.h>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <stdexcept>

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

/**
 * @brief 测试定时器回调抛出异常的情况
 */
void test_timer_callback_exception() {
    LOG_INFO("=== Test timer callback exception ===");
    EventLoop loop;
    std::atomic<int> count(0);

    // 创建一个会抛出异常的定时器
    loop.runAfter(0.5, [&count]() {
        count++;
        LOG_INFO("Timer triggered, count: %d", count.load());
        if (count.load() == 2) {
            LOG_INFO("Throwing exception from timer callback");
            throw std::runtime_error("Test exception from timer");
        }
    });

    // 创建一个周期性定时器，每0.3秒触发一次
    loop.runEvery(0.3, [&count]() {
        count++;
        LOG_INFO("Periodic timer triggered, count: %d", count.load());
    });

    // 2秒后退出循环
    loop.runAfter(2.0, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证定时器继续执行（异常被捕获）
    assert(count.load() >= 5); // 至少应该触发5次
    LOG_INFO("Timer callback exception test passed, total triggers: %d", count.load());
}

/**
 * @brief 测试大量定时器同时触发
 */
void test_massive_timers() {
    LOG_INFO("=== Test massive timers ===");
    EventLoop loop;
    std::atomic<int> count(0);
    const int NUM_TIMERS = 100;

    // 创建100个定时器，都在0.1秒后触发
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);

    std::vector<TimerId> timerIds;
    for (int i = 0; i < NUM_TIMERS; ++i) {
        TimerId timerId = loop.runAt(when, [&count]() {
            count++;
        });
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
}

/**
 * @brief 测试定时器过期时间在过去的情况
 */
void test_past_expiration() {
    LOG_INFO("=== Test timer with past expiration ===");
    EventLoop loop;
    std::atomic<bool> triggered(false);

    // 创建一个过期时间在过去的定时器
    Timestamp now = Timestamp::now();
    Timestamp past = addTime(now, -1.0); // 1秒前

    loop.runAt(past, [&triggered]() {
        triggered = true;
        LOG_INFO("Timer with past expiration triggered");
    });

    // 0.5秒后退出循环
    loop.runAfter(0.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证定时器被触发（应该立即触发）
    assert(triggered.load());
    LOG_INFO("Timer with past expiration was triggered immediately");
}

/**
 * @brief 测试极短间隔定时器
 */
void test_very_short_interval() {
    LOG_INFO("=== Test timer with very short interval ===");
    EventLoop loop;
    std::atomic<int> count(0);

    // 创建一个每0.001秒触发一次的定时器
    TimerId timerId = loop.runEvery(0.001, [&count]() {
        count++;
    });

    // 0.1秒后取消定时器
    loop.runAfter(0.1, [timerId, &loop]() {
        loop.cancel(timerId);
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
}

/**
 * @brief 测试定时器过期时间完全相同的情况
 */
void test_same_expiration() {
    LOG_INFO("=== Test timers with same expiration ===");
    EventLoop loop;
    std::atomic<int> count(0);

    // 创建3个过期时间完全相同的定时器
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);

    loop.runAt(when, [&count]() {
        count++;
        LOG_INFO("Timer 1 triggered");
    });

    loop.runAt(when, [&count]() {
        count++;
        LOG_INFO("Timer 2 triggered");
    });

    loop.runAt(when, [&count]() {
        count++;
        LOG_INFO("Timer 3 triggered");
    });

    // 0.5秒后退出循环
    loop.runAfter(0.5, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证所有定时器都触发了
    assert(count.load() == 3);
    LOG_INFO("All 3 timers with same expiration triggered");
}

/**
 * @brief 测试EventLoop退出时未触发定时器的清理
 */
void test_cleanup_on_exit() {
    LOG_INFO("=== Test cleanup on exit ===");

    {
        EventLoop loop;
        std::atomic<int> count(0);

        // 创建一个5秒后才触发的定时器
        loop.runAfter(5.0, [&count]() {
            count++;
            LOG_INFO("Long-delay timer triggered");
        });

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

    // EventLoop已析构，验证没有内存泄漏
    LOG_INFO("EventLoop destroyed successfully");
}

/**
 * @brief 测试定时器在执行过程中被取消的场景
 */
void test_cancel_during_execution() {
    LOG_INFO("=== Test cancel during execution ===");
    EventLoop loop;
    std::atomic<int> count(0);
    TimerId timerToCancel;

    // 创建一个每0.1秒触发一次的周期性定时器
    timerToCancel = loop.runEvery(0.1, [&count, &loop, &timerToCancel]() {
        count++;
        LOG_INFO("Timer triggered, count: %d", count.load());

        // 在第3次触发时取消自己
        if (count.load() == 3) {
            LOG_INFO("Canceling timer from its own callback");
            loop.cancel(timerToCancel);
        }
    });

    // 1秒后退出循环
    loop.runAfter(1.0, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 验证定时器只触发了3次
    assert(count.load() == 3);
    LOG_INFO("Timer was canceled during execution after 3 triggers");
}

/**
 * @brief 测试跨线程并发添加和取消定时器
 */
void test_concurrent_operations() {
    LOG_INFO("=== Test concurrent operations ===");
    EventLoop loop;
    std::atomic<int> count(0);
    std::atomic<int> canceled_count(0);
    const int NUM_THREADS = 5;
    const int TIMERS_PER_THREAD = 10;

    std::vector<std::thread> threads;
    std::vector<std::vector<TimerId>> allTimerIds(NUM_THREADS);

    // 启动多个线程，每个线程添加多个定时器
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([i, &loop, &allTimerIds]() {
            sleep(0.1); // 等待EventLoop启动

            // 添加多个定时器
            for (int j = 0; j < TIMERS_PER_THREAD; ++j) {
                TimerId timerId = loop.runAfter(0.1 + j * 0.05, []() {
                    LOG_INFO("Timer from thread triggered");
                });
                allTimerIds[i].push_back(timerId);
            }
        });
    }

    // 1秒后取消所有定时器
    loop.runAfter(1.0, [&loop, &allTimerIds, &canceled_count]() {
        for (const auto& timerIds : allTimerIds) {
            for (const auto& timerId : timerIds) {
                loop.cancel(timerId);
                canceled_count++;
            }
        }
    });

    // 2秒后退出循环
    loop.runAfter(2.0, [&loop]() {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证所有定时器都被取消
    assert(canceled_count.load() == NUM_THREADS * TIMERS_PER_THREAD);
    LOG_INFO("All %d timers were canceled", canceled_count.load());
}

int main() {
    LOG_INFO("=== Timer and EventLoop integration test ===");

    LOG_INFO("1. Test basic timer functionality");
    test_eventloop_timer();

    LOG_INFO("2. Test cross-thread timer");
    test_cross_thread_timer();

    LOG_INFO("3. Test timer callback exception");
    test_timer_callback_exception();

    LOG_INFO("4. Test massive timers");
    test_massive_timers();

    LOG_INFO("5. Test past expiration");
    test_past_expiration();

    LOG_INFO("6. Test very short interval");
    test_very_short_interval();

    LOG_INFO("7. Test same expiration");
    test_same_expiration();

    LOG_INFO("8. Test cleanup on exit");
    test_cleanup_on_exit();

    LOG_INFO("9. Test cancel during execution");
    test_cancel_during_execution();

    LOG_INFO("10. Test concurrent operations");
    test_concurrent_operations();

    LOG_INFO("All integration tests passed!");
    return 0;
}
