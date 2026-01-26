#include "Eventloop.h"
#include "Logger.h"
#include "Timer.h"
#include <thread>
#include <atomic>
#include <iostream>

void test_basic_timer()
{
    EventLoop loop;
    int count = 0;

    // 1秒后执行
    loop.runAfter(1.0, [&loop, &count] {
        LOG_INFO("Timer 1 triggered");
        count++;
    });

    // 每2秒执行一次
    auto timerId = loop.runEvery(2.0, [&loop, &count] {
        LOG_INFO("Periodic timer triggered, count: %d", count);
        if (++count >= 5) {
            // 取消定时器
            loop.quit();
        }
    });

    // 在指定时间执行
    auto now = Timestamp::now();
    auto when = addTime(now, 3.0);
    loop.runAt(when, [&loop] {
        LOG_INFO("Run at specific time");
    });

    loop.loop();
}

void test_timer_cancel()
{
    EventLoop loop;
    int count = 0;

    // 创建一个每0.5秒触发一次的定时器
    TimerId timerId = loop.runEvery(0.5, [&loop, &count] {
        LOG_INFO("Timer triggered, count: %d", count);
        count++;
    });

    // 2秒后取消定时器
    loop.runAfter(2.0, [&loop, timerId] {
        LOG_INFO("Cancel timer");
        loop.cancel(timerId);
    });

    // 3秒后退出循环
    loop.runAfter(3.0, [&loop] {
        LOG_INFO("Quit loop");
        loop.quit();
    });

    loop.loop();
}

int main()
{
    LOG_INFO("=== Test basic timer ===");
    test_basic_timer();

    LOG_INFO("=== Test timer cancel ===");
    test_timer_cancel();
    cout<< "all good"<< endl;
    return 0;
}
