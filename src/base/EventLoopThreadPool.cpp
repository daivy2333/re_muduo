#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include <assert.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg)
    : baseLoop_(baseLoop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // EventLoopThread 会自动析构，因为它们存储在 unique_ptr 中
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    assert(!started_);
    assert(baseLoop_->isInLoopThread());
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, string(buf));
        threads_.emplace_back(t);
        loops_.push_back(t->startLoop());
    }
    if (numThreads_ == 0 && cb)
    {
        // 只有一个线程（baseLoop），也需要调用回调
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    assert(baseLoop_->isInLoopThread());
    assert(started_);
    EventLoop *loop = baseLoop_;

    // 如果没有创建子线程，就使用 baseLoop
    if (!loops_.empty())
    {
        // round-robin 分配
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    assert(baseLoop_->isInLoopThread());
    assert(started_);
    if (loops_.empty())
    {
        return vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}
