#include "EventLoopThread.h"
#include "Eventloop.h"

/**
 * 构造函数
 * @param cb 线程初始化回调函数
 * @param name 线程名称
 */
EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const string &name)
    : loop_(nullptr),
      exiting_(false),
      thread(std::bind(&EventLoopThread::threadFunc, this), name),
      mutex_(),
      cond_(),
      callback_(cb)
{
}

/**
 * 析构函数
 */
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (loop_ != nullptr)
        {
            loop_->quit();
        }
    }
    thread.join();
}

/**
 * 启动事件循环线程
 * @return 返回事件循环对象的指针
 */
EventLoop* EventLoopThread::startLoop()
{
    // 启动线程
    thread.start();

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 等待线程创建并初始化 EventLoop
        while (loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

/**
 * 线程主函数
 * 在新线程中创建并运行事件循环
 */
void EventLoopThread::threadFunc()
{
    EventLoop loop;

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    // assert(exiting_);
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
