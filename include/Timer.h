#pragma once

#include "Callbacks.h"
#include "Timestamp.h"
#include <atomic>

/**
 * @brief Timer类，用于表示一个定时器
 *
 * Timer类封装了定时任务的基本信息，包括回调函数、过期时间、重复间隔等
 */
class Timer {
public:
    /**
     * @brief 构造函数
     * @param cb 定时器回调函数
     * @param when 过期时间
     * @param interval 重复间隔（秒），0表示不重复
     */
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(s_numCreated_++) {}

    /**
     * @brief 执行定时器回调
     */
    void run() const { callback_(); }

    /**
     * @brief 获取过期时间
     * @return 过期时间
     */
    Timestamp expiration() const { return expiration_; }

    /**
     * @brief 判断是否为重复定时器
     * @return true表示重复，false表示一次性
     */
    bool repeat() const { return repeat_; }

    /**
     * @brief 获取定时器序列号
     * @return 序列号
     */
    int64_t sequence() const { return sequence_; }

    /**
     * @brief 重启定时器
     * @param now 当前时间
     */
    void restart(Timestamp now);

    /**
     * @brief 获取已创建的定时器总数
     * @return 定时器总数
     */
    static int64_t numCreated() { return s_numCreated_.load(); }

private:
    const TimerCallback callback_;   ///< 定时器回调函数
    Timestamp expiration_;           ///< 过期时间
    const double interval_;          ///< 重复间隔（秒）
    const bool repeat_;              ///< 是否重复
    const int64_t sequence_;         ///< 序列号

    static std::atomic<int64_t> s_numCreated_;  ///< 已创建的定时器总数
};

/**
 * @brief TimerId类，用于标识和取消定时器
 *
 * TimerId是Timer的轻量级包装，用于取消定时器
 */
class TimerId {
public:
    TimerId() : timer_(nullptr), sequence_(0) {}
    TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}

private:
    Timer* timer_;      ///< 指向Timer对象的指针
    int64_t sequence_;  ///< 序列号

    friend class TimerQueue;
};
