#pragma once

#include "noncopyable.h"
#include<functional>
#include <mutex>
#include <condition_variable>
#include "Eventloop.h"
#include "Thread.h"
using namespace std;
/**
 * EventLoopThread 类，用于创建一个运行事件循环的线程
 * 该类不可复制(noncopyable)
 */
class EventLoopThread : noncopyable
{
public:
    // 定义线程初始化回调函数类型，参数为 EventLoop 指针
    using ThreadInitCallback = function<void(EventLoop*)>;

    /**
     * 构造函数
     * @param cb 线程初始化回调函数，默认为空函数
     * @param name 线程名称，默认为空字符串
     */
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
        const string &name = string());

    // 析构函数
    ~EventLoopThread();

    /**
     * 启动事件循环线程
     * @return 返回事件循环对象的指针
     */
    EventLoop* startLoop();
private:
    // 线程主函数
    void threadFunc();

    
    // 成员变量
    EventLoop *loop_;           // 事件循环指针
    bool exiting_;
    Thread thread;              // 线程对象
    mutex mutex_;               // 互斥锁
    condition_variable cond_;   // 条件变量
    ThreadInitCallback callback_; // 线程初始化回调函数
};