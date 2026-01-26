#pragma once

#include "Timer.h"
#include "Channel.h"
#include "noncopyable.h"
#include <set>
#include <vector>
#include <memory>

class EventLoop;

/**
 * @brief TimerQueue类，用于管理定时器队列
 *
 * TimerQueue使用timerfd和set来管理定时器，支持添加和取消定时器
 * 每个EventLoop有一个TimerQueue实例
 */
class TimerQueue : noncopyable {
public:
    /**
     * @brief 构造函数
     * @param loop 所属的EventLoop
     */
    explicit TimerQueue(EventLoop* loop);

    /**
     * @brief 析构函数
     */
    ~TimerQueue();

    /**
     * @brief 添加定时器
     * @param cb 定时器回调函数
     * @param when 过期时间
     * @param interval 重复间隔（秒）
     * @return TimerId，用于取消定时器
     */
    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

    /**
     * @brief 取消定时器
     * @param timerId 要取消的定时器ID
     */
    void cancel(TimerId timerId);

private:
    // 定时器条目类型
    typedef std::pair<Timestamp, Timer*> Entry;
    // 定时器列表类型
    typedef std::set<Entry> TimerList;
    // 活跃定时器类型
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    // 活跃定时器集合类型
    typedef std::set<ActiveTimer> ActiveTimerSet;

    /**
     * @brief 在IO线程中添加定时器
     * @param timer 要添加的定时器
     */
    void addTimerInLoop(Timer* timer);

    /**
     * @brief 在IO线程中取消定时器
     * @param timerId 要取消的定时器ID
     */
    void cancelInLoop(TimerId timerId);

    /**
     * @brief 处理timerfd的可读事件
     */
    void handleRead();

    /**
     * @brief 获取已过期的定时器
     * @param now 当前时间
     * @return 已过期的定时器列表
     */
    std::vector<Entry> getExpired(Timestamp now);

    /**
     * @brief 重置已过期的定时器
     * @param expired 已过期的定时器列表
     * @param now 当前时间
     */
    void reset(const std::vector<Entry>& expired, Timestamp now);

    /**
     * @brief 插入定时器到定时器集合
     * @param timer 要插入的定时器
     * @return true表示最早过期时间改变，false表示未改变
     */
    bool insert(Timer* timer);

    EventLoop* loop_;           ///< 所属的EventLoop
    const int timerfd_;         ///< timerfd文件描述符
    Channel timerfdChannel_;    ///< timerfd对应的Channel
    TimerList timers_;          ///< 定时器列表

    bool callingExpiredTimers_; ///< 是否正在执行过期定时器
    ActiveTimerSet activeTimers_;    ///< 活跃定时器集合
    ActiveTimerSet cancelingTimers_; ///< 正在取消的定时器集合
};
