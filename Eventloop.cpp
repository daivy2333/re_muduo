#include "Eventloop.h"
#include "Poller.h"
#include "Logger.h"
#include "TimerQueue.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>
#include <algorithm>

// 防止一个线程创建多个EventLoop
__thread EventLoop* t_loopInThisThread = 0;

// poll超时时间，单位毫秒
const int kPollTimeMs = 10000;

// 创建eventfd，用于线程唤醒
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("Failed in eventfd");
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr),
      timerQueue_(new TimerQueue(this))
{
    LOG_DEBUG("EventLoop created %p in thread %d", this, threadId_);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设置wakeupChannel的回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 监听wakeupFd的可读事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    // 注意：在析构时，looping_ 可能为 false
    // 因此不能直接调用 remove()，因为它要求 looping_ 为 true
    // 我们需要直接调用 Poller 的 removeChannel 方法
    wakeupChannel_->disableAll();
    // 直接从 Poller 中移除 Channel，而不通过 EventLoop::removeChannel
    poller_->removeChannel(wakeupChannel_.get());
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    // 不重置quit_标志，允许在调用loop()之前设置退出标志
    // quit_ = false;  // 注释掉这行，支持在调用loop()之前设置退出标志
    LOG_DEBUG("EventLoop %p start looping", this);

    while (!quit_)
    {
        activeChannels_.clear();
        // 调用poller等待IO事件
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        
        // 处理所有活跃的Channel
        for (Channel* channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = nullptr;

        // 执行待处理的回调函数
        doPendingFunctors();
    }

    LOG_DEBUG("EventLoop %p stop looping", this);
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // 如果不在IO线程中，需要唤醒IO线程
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        // 如果在IO线程中，直接执行
        cb();
    }
    else
    {
        // 如果不在IO线程中，排队到IO线程执行
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    // 如果不在IO线程中，或者正在执行待处理的回调，需要唤醒IO线程
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // 计数器已经非零，说明之前有未处理的唤醒事件
            // 这是正常情况，不需要记录错误
            LOG_DEBUG("EventLoop::wakeup() eventfd counter already non-zero");
        }
        else
        {
            LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8, errno=%d", n, errno);
        }
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    if (isInLoopThread())
    {
        poller_->updateChannel(channel);
    }
    else
    {
        // 在非IO线程中，排队到IO线程执行
        runInLoop(std::bind(&Poller::updateChannel, poller_.get(), channel));
    }
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    if (isInLoopThread())
    {
        // 移除对 looping_ 的断言要求，允许在非循环状态下移除 Channel
        if (currentActiveChannel_ == channel)
        {
            // 正在处理这个channel，不能删除
            assert(false);
        }
        poller_->removeChannel(channel);
    }
    else
    {
        // 在非IO线程中，排队到IO线程执行
        runInLoop(std::bind(&EventLoop::removeChannelInLoop, this, channel));
    }
}

bool EventLoop::hasChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assert(isInLoopThread());
    return poller_->hasChannel(channel);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // 计数器为0，说明没有待处理的唤醒事件
            // 这是正常情况，不需要记录错误
            LOG_DEBUG("EventLoop::handleRead() eventfd counter is zero");
        }
        else
        {
            LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8, errno=%d", n, errno);
        }
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors)
    {
        functor();
    }

    callingPendingFunctors_ = false;
}

void EventLoop::removeChannelInLoop(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assert(isInLoopThread());
    // 移除对 looping_ 的断言要求，允许在非循环状态下移除 Channel
    if (currentActiveChannel_ == channel)
    {
        // 正在处理这个channel，不能删除
        assert(false);
    }
    poller_->removeChannel(channel);
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
    timerQueue_->cancel(timerId);
}