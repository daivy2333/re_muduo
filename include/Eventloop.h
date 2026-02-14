#pragma once

#include "noncopyable.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include "assert.h"
// 前向声明，避免循环依赖
// Eventloop.h 和 Poller.h 相互包含，需要使用前向声明
class Poller;
class TimerQueue;

class TimerId;

// EventLoop: 事件循环类，负责管理IO事件和定时任务
// 每个线程最多有一个EventLoop实例
// 主要功能：
// 1. 通过Poller监听文件描述符上的IO事件
// 2. 管理Channel对象，处理事件回调
// 3. 支持跨线程任务调度
// 4. 支持定时器任务（通过TimerQueue）
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // 启动事件循环，必须在创建EventLoop的线程中调用
    void loop();

    // 设置进入事件循环的回调函数
    void setLoopStartedCallback(std::function<void()> cb) { loopStartedCallback_ = cb; }
    
    // 退出事件循环，可以在其他线程安全调用
    void quit();
    
    // 返回Poller返回的时间戳（即最近一次IO事件发生的时间）
    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前IO线程中执行回调，如果不在IO线程则排队到IO线程执行
    void runInLoop(Functor cb);
    
    // 将回调函数排队到IO线程执行
    void queueInLoop(Functor cb);

    // 唤醒IO线程（如果当前线程阻塞在Poller上）
    void wakeup();

    // 更新Channel在Poller上的关注事件
    void updateChannel(Channel* channel);
    
    // 从Poller中移除Channel
    void removeChannel(Channel* channel);
    
    // 检查Channel是否已注册到Poller
    bool hasChannel(Channel* channel);

    // 获取EventLoop内部的Poller（用于测试）
    Poller* getPoller() { return poller_.get(); }

    // 判断当前线程是否是创建EventLoop的线程
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    // 断言当前线程是创建EventLoop的线程
    void assertInLoopThread() { assert(isInLoopThread()); }

    // 定时器相关接口
    // 在指定时间点运行回调
    TimerId runAt(Timestamp time, TimerCallback cb);
    // 在延迟delay秒后运行回调
    TimerId runAfter(double delay, TimerCallback cb);
    // 每隔interval秒运行回调
    TimerId runEvery(double interval, TimerCallback cb);
    // 取消定时器
    void cancel(TimerId timerId);

private:
    // 处理wakeupfd上的可读事件
    void handleRead();
    
    // 执行待处理的回调函数
    void doPendingFunctors();

    // 在IO线程中移除Channel
    void removeChannelInLoop(Channel* channel);

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;              // 是否正在事件循环中
    std::atomic_bool quit_;                 // 是否请求退出循环
    std::atomic_bool callingPendingFunctors_; // 是否正在执行待处理的回调
    const pid_t threadId_;                  // 创建EventLoop的线程ID
    Timestamp pollReturnTime_;              // Poller返回的时间戳
    std::unique_ptr<Poller> poller_;        // IO多路复用器

    int wakeupFd_;                          // 用于唤醒IO线程的eventfd
    std::unique_ptr<Channel> wakeupChannel_; // wakeupFd对应的Channel

    ChannelList activeChannels_;            // 活跃的Channel列表
    Channel* currentActiveChannel_;         // 当前正在处理的Channel

    std::vector<Functor> pendingFunctors_;  // 待执行的回调函数队列
    std::mutex mutex_;                      // 保护pendingFunctors_的互斥锁
    std::unique_ptr<TimerQueue> timerQueue_; // 定时器队列
    std::function<void()> loopStartedCallback_; // 进入事件循环的回调函数
};