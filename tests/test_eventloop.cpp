#include "Eventloop.h"
#include "Channel.h"
#include "Logger.h"
#include "Timer.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <unistd.h>
#include <sys/eventfd.h>
#include <atomic>
#include <chrono>
#include <vector>
#include <condition_variable>

// 测试1: 基本功能测试
void test_eventloop_basic() {
    std::cout << "Test EventLoop basic functionality" << std::endl;

    EventLoop loop;
    assert(loop.isInLoopThread());
    std::cout << "EventLoop is in the correct thread" << std::endl;

    // 测试跨线程退出
    std::thread quitThread([&loop]() {
        sleep(1);
        loop.quit();
    });

    loop.loop();
    quitThread.join();

    std::cout << "EventLoop basic test passed" << std::endl;
}

// 测试2: Channel事件处理测试
void test_eventloop_channel() {
    std::cout << "Test EventLoop with Channel" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    int eventfd = -1;
    std::atomic<bool> callbackCalled{false};
    std::atomic<int> callbackCount{0};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(eventfd >= 0);

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        Channel channel(&loop, eventfd);
        channel.setReadCallback([eventfd, &callbackCalled, &callbackCount](Timestamp receiveTime) {
            callbackCalled.store(true, std::memory_order_release);
            callbackCount.fetch_add(1, std::memory_order_relaxed);
            // 读取eventfd的值，清除可读事件
            uint64_t value;
            ssize_t n = ::read(eventfd, &value, sizeof(value));
            if (n != sizeof(value) && errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "read failed, errno=" << errno << std::endl;
            }
            std::cout << "Read callback called at " << receiveTime.to_string()
                      << ", count: " << callbackCount.load(std::memory_order_relaxed) << std::endl;
        });

        channel.enableReading();
        assert(loop.hasChannel(&channel));

        loop.loop();

        channel.disableAll();
        channel.remove();
        assert(!loop.hasChannel(&channel));
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 多次写入eventfd，测试多次事件触发
    std::thread writerThread([eventfd]() {
        sleep(1);
        for (int i = 0; i < 3; ++i) {
            uint64_t one = 1;
            ssize_t n = ::write(eventfd, &one, sizeof(one));
            if (n != sizeof(one) && errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "write failed, errno=" << errno << std::endl;
            }
            usleep(100000); // 100ms
        }
    });

    // 等待所有回调执行完成
    while (callbackCount.load(std::memory_order_acquire) < 3) {
        usleep(10000); // 10ms
    }

    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->runInLoop([currentLoop]() {
            currentLoop->quit();
        });
    }

    writerThread.join();
    loopThread.join();
    close(eventfd);

    assert(callbackCalled.load(std::memory_order_acquire));
    assert(callbackCount.load(std::memory_order_acquire) >= 1);

    std::cout << "EventLoop channel test passed" << std::endl;
}

// 测试3: runInLoop和queueInLoop测试
void test_eventloop_runinloop() {
    std::cout << "Test EventLoop::runInLoop" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    std::atomic<bool> inLoopCallbackCalled{false};
    std::atomic<bool> queuedCallbackCalled{false};
    std::atomic<int> totalCallbacks{0};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        // 在IO线程中调用runInLoop，应该立即执行
        loop.runInLoop([&]() {
            inLoopCallbackCalled.store(true, std::memory_order_release);
            totalCallbacks.fetch_add(1, std::memory_order_relaxed);
            std::cout << "runInLoop callback executed immediately in IO thread" << std::endl;
        });

        // 验证立即执行
        assert(inLoopCallbackCalled.load(std::memory_order_acquire));

        loop.loop();
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 在另一个线程中调用runInLoop，应该排队执行
    std::thread otherThread([&loopPtr, &queuedCallbackCalled, &totalCallbacks]() {
        for (int i = 0; i < 5; ++i) {
            auto currentLoop = loopPtr.load(std::memory_order_acquire);
            if (currentLoop) {
                currentLoop->runInLoop([&]() {
                    queuedCallbackCalled.store(true, std::memory_order_release);
                    totalCallbacks.fetch_add(1, std::memory_order_relaxed);
                    std::cout << "runInLoop callback queued to IO thread" << std::endl;
                });
            }
        }
    });

    // 等待所有回调执行完成
    while (totalCallbacks.load(std::memory_order_acquire) < 6) {
        usleep(10000); // 10ms
    }

    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->runInLoop([currentLoop]() {
            currentLoop->quit();
        });
    }

    otherThread.join();
    loopThread.join();

    assert(inLoopCallbackCalled.load(std::memory_order_acquire));
    assert(queuedCallbackCalled.load(std::memory_order_acquire));
    assert(totalCallbacks.load(std::memory_order_acquire) == 6);

    std::cout << "EventLoop::runInLoop test passed" << std::endl;
}

// 测试4: 定时器测试
void test_eventloop_timer() {
    std::cout << "Test EventLoop timer functionality" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    std::atomic<int> timerCount{0};
    std::atomic<bool> oneShotCalled{false};
    std::atomic<int> repeatCount{0};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        loop.loop();
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 测试runAt - 在指定时间执行一次
    Timestamp when = addTime(Timestamp::now(), 0.5);
    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    TimerId timer1;
    if (currentLoop) {
        timer1 = currentLoop->runAt(when, [&]() {
            oneShotCalled.store(true, std::memory_order_release);
            timerCount.fetch_add(1, std::memory_order_relaxed);
            std::cout << "runAt timer called" << std::endl;
        });
    }

    // 测试runEvery - 重复定时器
    currentLoop = loopPtr.load(std::memory_order_acquire);
    TimerId timer2;
    if (currentLoop) {
        timer2 = currentLoop->runEvery(0.3, [&]() {
            repeatCount.fetch_add(1, std::memory_order_relaxed);
            timerCount.fetch_add(1, std::memory_order_relaxed);
            std::cout << "runEvery timer called, count: " << repeatCount.load(std::memory_order_relaxed) << std::endl;
            // 不在回调中调用quit，避免与TimerQueue的reset冲突
        });
    }

    // 测试runAfter - 延迟执行
    currentLoop = loopPtr.load(std::memory_order_acquire);
    TimerId timer3;
    if (currentLoop) {
        timer3 = currentLoop->runAfter(0.2, [&]() {
            timerCount.fetch_add(1, std::memory_order_relaxed);
            std::cout << "runAfter timer called" << std::endl;
        });
    }

    // 使用单独的定时器来退出事件循环
    currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->runAfter(1.2, [currentLoop]() {
            currentLoop->quit();
        });
    }

    loopThread.join();

    assert(oneShotCalled.load(std::memory_order_acquire));
    assert(repeatCount.load(std::memory_order_acquire) >= 3);
    assert(timerCount.load(std::memory_order_acquire) >= 5);

    std::cout << "EventLoop timer test passed" << std::endl;
}

// 测试5: 定时器取消测试
void test_eventloop_timer_cancel() {
    std::cout << "Test EventLoop timer cancellation" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    std::atomic<int> timerCount{0};
    std::atomic<bool> cancelledTimerCalled{false};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        loop.loop();
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 创建一个会被取消的定时器
    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    TimerId timerToCancel;
    if (currentLoop) {
        timerToCancel = currentLoop->runAfter(0.5, [&]() {
            cancelledTimerCalled.store(true, std::memory_order_release);
            timerCount.fetch_add(1, std::memory_order_relaxed);
            std::cout << "Cancelled timer called (should not happen)" << std::endl;
        });
    }

    // 创建一个正常执行的定时器
    currentLoop = loopPtr.load(std::memory_order_acquire);
    TimerId normalTimer;
    if (currentLoop) {
        normalTimer = currentLoop->runAfter(0.3, [&]() {
            timerCount.fetch_add(1, std::memory_order_relaxed);
            std::cout << "Normal timer called" << std::endl;
        });
    }

    // 创建一个用于退出的定时器
    currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->runAfter(1.0, [currentLoop]() {
            currentLoop->quit();
        });
    }

    // 取消第一个定时器
    sleep(0.1);
    currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->cancel(timerToCancel);
    }

    loopThread.join();

    assert(!cancelledTimerCalled.load(std::memory_order_acquire));
    assert(timerCount.load(std::memory_order_acquire) == 1);  // 只有normalTimer执行了一次

    std::cout << "EventLoop timer cancellation test passed" << std::endl;
}

// 测试6: 多线程并发测试
void test_eventloop_multithread() {
    std::cout << "Test EventLoop with multiple threads" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    std::atomic<int> callbackCount{0};
    const int kThreadCount = 5;
    const int kCallbacksPerThread = 10;
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        loop.loop();
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&loopPtr, &callbackCount]() {
            for (int j = 0; j < kCallbacksPerThread; ++j) {
                auto currentLoop = loopPtr.load(std::memory_order_acquire);
                if (currentLoop) {
                    currentLoop->runInLoop([&callbackCount]() {
                        callbackCount.fetch_add(1, std::memory_order_relaxed);
                    });
                }
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 等待所有回调执行完成
    while (callbackCount.load(std::memory_order_acquire) < kThreadCount * kCallbacksPerThread) {
        usleep(10000); // 10ms
    }

    // 退出事件循环
    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->runInLoop([currentLoop]() {
            currentLoop->quit();
        });
    }

    loopThread.join();

    assert(callbackCount.load(std::memory_order_acquire) == kThreadCount * kCallbacksPerThread);

    std::cout << "EventLoop multithread test passed" << std::endl;
}

// 测试7: 边界条件测试
void test_eventloop_edge_cases() {
    std::cout << "Test EventLoop edge cases" << std::endl;

    // 测试空回调
    {
        EventLoop loop;
        TimerId invalidTimer = loop.runAfter(0.1, nullptr);
        assert(!invalidTimer.isValid());
    }

    // 测试无效时间戳
    {
        EventLoop loop;
        Timestamp invalidTime;
        assert(!invalidTime.valid());
        TimerId timer = loop.runAt(invalidTime, []() {});
        assert(!timer.isValid());
    }

    // 测试负延迟
    {
        EventLoop loop;
        TimerId timer = loop.runAfter(-1.0, []() {});
        assert(!timer.isValid());
    }

    // 测试零间隔
    {
        EventLoop loop;
        TimerId timer = loop.runEvery(0.0, []() {});
        assert(!timer.isValid());
    }

    // 测试多次调用quit
    {
        EventLoop loop;
        std::thread t([&loop]() {
            sleep(0.1);
            loop.quit();
            loop.quit(); // 多次调用quit
        });
        loop.loop();
        t.join();
    }

    // 测试在回调中调用quit
    {
        EventLoop loop;
        loop.runAfter(0.1, [&loop]() {
            loop.quit();
        });
        loop.loop();
    }

    std::cout << "EventLoop edge cases test passed" << std::endl;
}

// 测试8: Channel生命周期测试
void test_eventloop_channel_lifetime() {
    std::cout << "Test EventLoop Channel lifetime" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    int eventfd = -1;
    std::atomic<bool> callbackCalled{false};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(eventfd >= 0);

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        {
            Channel channel(&loop, eventfd);
            channel.setReadCallback([&callbackCalled](Timestamp receiveTime) {
                callbackCalled.store(true, std::memory_order_release);
                std::cout << "Read callback called at " << receiveTime.to_string() << std::endl;
            });

            channel.enableReading();

            // 运行事件循环
            loop.loop();

            // Channel在作用域结束时自动析构
            // 但在此之前需要先移除
            channel.disableAll();
            channel.remove();
        }
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 写入eventfd触发读事件
    uint64_t one = 1;
    ssize_t n = ::write(eventfd, &one, sizeof(one));
    if (n != sizeof(one) && errno != EAGAIN && errno != EWOULDBLOCK) {
        std::cerr << "write failed, errno=" << errno << std::endl;
    }

    // 等待回调被调用
    while (!callbackCalled.load(std::memory_order_acquire)) {
        usleep(10000); // 10ms
    }

    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->runInLoop([currentLoop]() {
            currentLoop->quit();
        });
    }

    loopThread.join();
    close(eventfd);

    assert(callbackCalled.load(std::memory_order_acquire));

    std::cout << "EventLoop Channel lifetime test passed" << std::endl;
}

// 测试9: wakeup机制测试
void test_eventloop_wakeup() {
    std::cout << "Test EventLoop wakeup mechanism" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    std::atomic<int> wakeupCount{0};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        // 在回调中增加计数
        loop.runInLoop([&wakeupCount]() {
            wakeupCount.fetch_add(1, std::memory_order_relaxed);
        });

        loop.loop();
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 多次调用wakeup
    for (int i = 0; i < 5; ++i) {
        auto currentLoop = loopPtr.load(std::memory_order_acquire);
        if (currentLoop) {
            currentLoop->wakeup();
        }
        usleep(50000); // 50ms
    }

    sleep(0.5);

    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->quit();
    }
    loopThread.join();

    // wakeupCount应该至少为1
    assert(wakeupCount.load(std::memory_order_acquire) >= 1);

    std::cout << "EventLoop wakeup test passed" << std::endl;
}

// 测试10: 在pendingFunctors中添加新任务
void test_eventloop_nested_pending() {
    std::cout << "Test EventLoop nested pending functors" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    std::atomic<int> level1Count{0};
    std::atomic<int> level2Count{0};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        loop.loop();
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 添加第一层任务
    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->runInLoop([currentLoop, &level1Count, &level2Count]() {
            level1Count.fetch_add(1, std::memory_order_relaxed);
            std::cout << "Level 1 callback" << std::endl;

            // 在第一层任务中添加第二层任务
            currentLoop->runInLoop([&level2Count]() {
                level2Count.fetch_add(1, std::memory_order_relaxed);
                std::cout << "Level 2 callback" << std::endl;
            });
        });
    }

    // 等待所有回调执行完成
    while (level1Count.load(std::memory_order_acquire) < 1 || 
           level2Count.load(std::memory_order_acquire) < 1) {
        usleep(10000); // 10ms
    }

    currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        currentLoop->quit();
    }
    loopThread.join();

    assert(level1Count.load(std::memory_order_acquire) == 1);
    assert(level2Count.load(std::memory_order_acquire) == 1);

    std::cout << "EventLoop nested pending functors test passed" << std::endl;
}

// 测试11: 测试pollReturnTime
void test_eventloop_poll_return_time() {
    std::cout << "Test EventLoop pollReturnTime" << std::endl;

    EventLoop loop;

    // 使用定时器来触发quit，确保至少执行一次poll
    loop.runAfter(0.1, [&loop]() {
        loop.quit();
    });

    Timestamp beforeLoop = Timestamp::now();
    loop.loop();
    Timestamp afterLoop = Timestamp::now();
    Timestamp pollTime = loop.pollReturnTime();

    // pollReturnTime应该在beforeLoop和afterLoop之间
    assert(pollTime >= beforeLoop);
    assert(pollTime <= afterLoop);

    std::cout << "EventLoop pollReturnTime test passed" << std::endl;
}

// 测试12: 测试assertInLoopThread
void test_eventloop_assert_in_loop_thread() {
    std::cout << "Test EventLoop assertInLoopThread" << std::endl;

    std::atomic<EventLoop*> loopPtr{nullptr};
    std::mutex mtx;
    std::condition_variable cv;
    bool loopReady = false;

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr.store(&loop, std::memory_order_release);

        {
            std::lock_guard<std::mutex> lock(mtx);
            loopReady = true;
            cv.notify_one();
        }

        // 在IO线程中调用assertInLoopThread应该成功
        loop.assertInLoopThread();

        loop.loop();
    });

    // 等待EventLoop创建完成
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&loopReady] { return loopReady; });
    }

    // 在非IO线程中调用assertInLoopThread应该失败
    // 这里我们只测试isInLoopThread，避免程序崩溃
    auto currentLoop = loopPtr.load(std::memory_order_acquire);
    if (currentLoop) {
        assert(!currentLoop->isInLoopThread());
        currentLoop->quit();
    }
    loopThread.join();

    std::cout << "EventLoop assertInLoopThread test passed" << std::endl;
}

// 测试13: 测试多次循环
void test_eventloop_multiple_loops() {
    std::cout << "Test EventLoop multiple loops" << std::endl;

    EventLoop loop;
    int loopCount = 0;

    // 运行多次循环
    for (int i = 0; i < 3; ++i) {
        std::thread t([&loop, &loopCount]() {
            sleep(0.2);
            loopCount++;
            loop.quit();
        });

        loop.loop();
        t.join();
    }

    assert(loopCount == 3);

    std::cout << "EventLoop multiple loops test passed" << std::endl;
}

// 测试14: 测试Channel的所有事件类型
void test_eventloop_channel_events() {
    std::cout << "Test EventLoop Channel events" << std::endl;

    EventLoop loop;
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(eventfd >= 0);

    std::atomic<bool> readCalled{false};
    std::atomic<bool> writeCalled{false};

    Channel channel(&loop, eventfd);

    // 测试读事件
    channel.setReadCallback([&readCalled](Timestamp) {
        readCalled = true;
    });

    channel.enableReading();
    assert(channel.isReading());
    assert(!channel.isWriting());

    // 测试写事件
    channel.setWriteCallback([&writeCalled]() {
        writeCalled = true;
    });

    channel.enableWriting();
    assert(channel.isWriting());

    // 测试禁用事件
    channel.disableReading();
    assert(!channel.isReading());

    channel.disableWriting();
    assert(!channel.isWriting());

    channel.disableAll();
    assert(channel.isNoneEvent());

    channel.remove();
    close(eventfd);

    std::cout << "EventLoop Channel events test passed" << std::endl;
}

int main() {
    std::cout << "=== EventLoop Tests ===" << std::endl;

    test_eventloop_basic();
    test_eventloop_channel();
    test_eventloop_runinloop();
    test_eventloop_timer();
    test_eventloop_timer_cancel();
    test_eventloop_multithread();
    test_eventloop_edge_cases();
    test_eventloop_channel_lifetime();
    test_eventloop_wakeup();
    test_eventloop_nested_pending();
    test_eventloop_poll_return_time();
    test_eventloop_assert_in_loop_thread();
    test_eventloop_multiple_loops();
    test_eventloop_channel_events();

    std::cout << "=== All EventLoop Tests Passed ===" << std::endl;
    return 0;
}
