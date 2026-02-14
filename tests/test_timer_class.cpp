#include "Timer.h"
#include "Timestamp.h"
#include "Logger.h"
#include <cassert>
#include <iostream>
#include <exception>
#include <stdexcept>

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

/**
 * @brief 测试Timer回调抛出异常的情况
 */
void test_timer_callback_exception()
{
    LOG_INFO("=== Test Timer callback exception ===");

    bool exceptionCaught = false;
    int callbackCount = 0;

    TimerCallback cb = [&callbackCount]() {
        callbackCount++;
        if (callbackCount == 2) {
            LOG_INFO("Timer callback throwing exception");
            throw std::runtime_error("Test exception");
        }
    };

    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);
    Timer timer(cb, when, 0.0);

    // 第一次调用应该成功
    try {
        timer.run();
        assert(callbackCount == 1);
    } catch (...) {
        assert(false); // 不应该抛出异常
    }

    // 第二次调用应该抛出异常
    try {
        timer.run();
        assert(false); // 不应该到达这里
    } catch (const std::exception& e) {
        exceptionCaught = true;
        assert(callbackCount == 2);
        LOG_INFO("Caught expected exception: %s", e.what());
    }

    assert(exceptionCaught);

    LOG_INFO("Timer callback exception test passed");
}

/**
 * @brief 测试无效时间戳的处理
 */
void test_timer_invalid_timestamp()
{
    LOG_INFO("=== Test Timer with invalid timestamp ===");

    TimerCallback cb = []() {
        LOG_INFO("Timer callback");
    };

    // 创建一个无效时间戳
    Timestamp invalid = Timestamp::invalid();

    // 使用无效时间戳创建定时器
    Timer timer(cb, invalid, 0.0);

    // 验证过期时间无效
    assert(!timer.expiration().valid());

    // 测试restart时传入无效时间戳
    Timestamp now = Timestamp::now();
    Timer periodicTimer(cb, addTime(now, 0.1), 0.2);

    periodicTimer.restart(invalid);

    // 验证周期性定时器在无效时间戳下也变为无效
    assert(!periodicTimer.expiration().valid());

    LOG_INFO("Timer invalid timestamp test passed");
}

/**
 * @brief 测试Timer的空回调
 */
void test_timer_null_callback()
{
    LOG_INFO("=== Test Timer with null callback ===");

    // 创建一个空回调的定时器
    TimerCallback cb = nullptr;

    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);

    Timer timer(cb, when, 0.0);

    // 调用run不应该崩溃
    timer.run();

    LOG_INFO("Timer null callback test passed");
}

/**
 * @brief 测试极短间隔的定时器
 */
void test_timer_very_short_interval()
{
    LOG_INFO("=== Test Timer with very short interval ===");

    int callbackCount = 0;
    TimerCallback cb = [&callbackCount]() {
        callbackCount++;
    };

    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.001); // 1毫秒

    Timer timer(cb, when, 0.001); // 1毫秒间隔

    // 验证是周期性定时器
    assert(timer.repeat());

    // 测试多次restart
    for (int i = 0; i < 10; ++i) {
        Timestamp newTime = Timestamp::now();
        timer.restart(newTime);
        assert(timer.expiration().valid());
    }

    LOG_INFO("Timer very short interval test passed");
}

/**
 * @brief 测试Timer的负间隔
 */
void test_timer_negative_interval()
{
    LOG_INFO("=== Test Timer with negative interval ===");

    TimerCallback cb = []() {
        LOG_INFO("Timer callback");
    };

    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);

    // 创建一个负间隔的定时器（应该被视为非重复定时器）
    Timer timer(cb, when, -0.1);

    // 验证不是重复定时器
    assert(!timer.repeat());

    // 测试restart
    Timestamp newTime = Timestamp::now();
    timer.restart(newTime);

    // 验证过期时间无效
    assert(!timer.expiration().valid());

    LOG_INFO("Timer negative interval test passed");
}

/**
 * @brief 测试Timer的零间隔
 */
void test_timer_zero_interval()
{
    LOG_INFO("=== Test Timer with zero interval ===");

    TimerCallback cb = []() {
        LOG_INFO("Timer callback");
    };

    Timestamp now = Timestamp::now();
    Timestamp when = addTime(now, 0.1);

    // 创建一个零间隔的定时器（应该被视为非重复定时器）
    Timer timer(cb, when, 0.0);

    // 验证不是重复定时器
    assert(!timer.repeat());

    // 测试restart
    Timestamp newTime = Timestamp::now();
    timer.restart(newTime);

    // 验证过期时间无效
    assert(!timer.expiration().valid());

    LOG_INFO("Timer zero interval test passed");
}

int main()
{
    test_timer_basic();
    test_timer_sequence();
    test_timer_expiration();
    test_timer_callback_exception();
    test_timer_invalid_timestamp();
    test_timer_null_callback();
    test_timer_very_short_interval();
    test_timer_negative_interval();
    test_timer_zero_interval();

    LOG_INFO("All Timer class tests passed!");
    return 0;
}
