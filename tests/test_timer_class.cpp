#include "Timer.h"
#include "Timestamp.h"
#include "Logger.h"
#include <cassert>
#include <iostream>

/**
 * @brief 测试Timer类的基本功能
 * 
 * 测试内容：
 * 1. 创建一次性定时器
 * 2. 创建周期性定时器
 * 3. 测试restart方法
 * 4. 验证定时器属性（过期时间、重复间隔、序列号等）
 */
void test_timer_basic()
{
    LOG_INFO("=== Test Timer basic functionality ===");

    int callbackCount = 0;
    TimerCallback cb = [&callbackCount]() {
        callbackCount++;
        LOG_INFO("Timer callback triggered, count: %d", callbackCount);
    };

    // 1. 创建一次性定时器
    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 1.0);
    Timer oneShotTimer(cb, when, 0.0);

    assert(!oneShotTimer.repeat()); // 应该不是重复定时器
    assert(oneShotTimer.expiration() == when); // 过期时间应该正确
    assert(oneShotTimer.sequence() >= 0); // 序列号应该有效

    LOG_INFO("One-shot timer created, expiration: %s", 
             oneShotTimer.expiration().to_string().c_str());

    // 2. 创建周期性定时器
    Timestamp now2 = Timestamp::now();
    Timestamp when2 = addTime(now2, 0.5);
    Timer periodicTimer(cb, when2, 0.5);

    assert(periodicTimer.repeat()); // 应该是重复定时器
    assert(periodicTimer.expiration() == when2); // 过期时间应该正确
    assert(periodicTimer.sequence() > oneShotTimer.sequence()); // 序列号应该递增

    LOG_INFO("Periodic timer created, expiration: %s, interval: 0.5s",
             periodicTimer.expiration().to_string().c_str());

    // 3. 测试run方法
    oneShotTimer.run();
    assert(callbackCount == 1);

    periodicTimer.run();
    assert(callbackCount == 2);

    // 4. 测试restart方法
    Timestamp newTime = Timestamp::now();

    // 对于一次性定时器，restart应该设置过期时间为无效
    oneShotTimer.restart(newTime);
    assert(!oneShotTimer.expiration().valid()); // 一次性定时器重启后应该无效

    // 对于周期性定时器，restart应该更新过期时间
    Timestamp oldExpiration = periodicTimer.expiration();
    periodicTimer.restart(newTime);
    assert(periodicTimer.expiration() > oldExpiration); // 新过期时间应该晚于旧时间
    assert(periodicTimer.expiration().valid()); // 周期性定时器重启后应该有效

    LOG_INFO("Timer restart test passed");

    // 5. 测试静态方法
    int64_t numCreated = Timer::numCreated();
    assert(numCreated >= 2); // 至少创建了2个定时器

    LOG_INFO("Total timers created: %ld", numCreated);

    LOG_INFO("Timer basic test passed");
}

/**
 * @brief 测试Timer的序列号生成
 * 
 * 测试内容：
 * 1. 创建多个定时器
 * 2. 验证序列号是递增的
 * 3. 验证numCreated统计正确
 */
void test_timer_sequence()
{
    LOG_INFO("=== Test Timer sequence generation ===");

    TimerCallback cb = []() {
        LOG_INFO("Timer callback");
    };

    int64_t startNumCreated = Timer::numCreated();
    std::vector<int64_t> sequences;

    // 创建10个定时器
    for (int i = 0; i < 10; ++i)
    {
        Timestamp now = Timestamp::now();
        Timestamp when = addTime(now, i * 0.1);
        Timer timer(cb, when, 0.0);
        sequences.push_back(timer.sequence());
    }

    // 验证序列号递增
    for (size_t i = 1; i < sequences.size(); ++i)
    {
        assert(sequences[i] > sequences[i-1]);
    }

    // 验证创建数量
    assert(Timer::numCreated() == startNumCreated + 10);

    LOG_INFO("Created 10 timers, sequences are monotonically increasing");
    LOG_INFO("Total timers created: %ld", Timer::numCreated());

    LOG_INFO("Timer sequence test passed");
}

/**
 * @brief 测试Timer的过期时间计算
 * 
 * 测试内容：
 * 1. 创建不同延迟的定时器
 * 2. 验证过期时间计算正确
 * 3. 测试restart对过期时间的影响
 */
void test_timer_expiration()
{
    LOG_INFO("=== Test Timer expiration calculation ===");

    TimerCallback cb = []() {
        LOG_INFO("Timer callback");
    };

    Timestamp now = Timestamp::now();

    // 创建不同延迟的定时器
    Timer timer1(cb, addTime(now, 0.1), 0.0);
    Timer timer2(cb, addTime(now, 0.5), 0.0);
    Timer timer3(cb, addTime(now, 1.0), 0.0);

    // 验证过期时间顺序
    assert(timer1.expiration() < timer2.expiration());
    assert(timer2.expiration() < timer3.expiration());

    // 验证时间差
    double diff12 = timeDifference(timer2.expiration(), timer1.expiration());
    double diff23 = timeDifference(timer3.expiration(), timer2.expiration());

    assert(diff12 >= 0.39 && diff12 <= 0.41); // 约0.4秒
    assert(diff23 >= 0.49 && diff23 <= 0.51); // 约0.5秒

    LOG_INFO("Expiration times are correctly ordered");

    // 测试周期性定时器的restart
    Timer periodicTimer(cb, addTime(now, 0.1), 0.2);
    Timestamp oldExpiration = periodicTimer.expiration();

    Timestamp newTime = addTime(oldExpiration, 0.3);
    periodicTimer.restart(newTime);

    Timestamp newExpiration = periodicTimer.expiration();
    double diff = timeDifference(newExpiration, newTime);

    assert(diff >= 0.19 && diff <= 0.21); // 约0.2秒（周期）

    LOG_INFO("Periodic timer restart correctly updates expiration");

    LOG_INFO("Timer expiration test passed");
}

int main()
{
    test_timer_basic();
    test_timer_sequence();
    test_timer_expiration();

    LOG_INFO("All Timer class tests passed!");
    return 0;
}
