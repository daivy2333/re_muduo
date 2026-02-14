#include "TimerQueue.h"
#include "Eventloop.h"
#include "Logger.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>

namespace {

// 创建timerfd
int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_FATAL("Failed in timerfd_create");
    }
    return timerfd;
}

// 计算从现在到指定时间的间隔
struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

// 重置timerfd
void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        int savedErrno = errno;
        LOG_ERROR("timerfd_settime() failed, errno=%d", savedErrno);
        // 如果timerfd无效，记录严重错误
        if (savedErrno == EBADF || savedErrno == EINVAL)
        {
            LOG_FATAL("resetTimerfd() timerfd is invalid, fd=%d", timerfd);
        }
    }
}

// 读取timerfd
void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    LOG_DEBUG("TimerQueue::handleRead() %lu at %s", howmany, now.to_string().c_str());
    if (n != sizeof(howmany))
    {
        int savedErrno = errno;
        LOG_ERROR("TimerQueue::handleRead() reads %ld bytes instead of 8, errno=%d", n, savedErrno);
        // 如果timerfd无效，记录严重错误
        if (savedErrno == EBADF || savedErrno == EINVAL)
        {
            LOG_FATAL("readTimerfd() timerfd is invalid, fd=%d", timerfd);
        }
    }
}

} // namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    for (const Entry& it : timers_)
    {
        delete it.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    // 检查回调函数是否为空
    if (!cb)
    {
        LOG_ERROR("TimerQueue::addTimer() callback is empty");
        return TimerId();
    }

    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    loop_->assertInLoopThread();

    // 检查timer是否有效
    if (timer == nullptr)
    {
        LOG_ERROR("TimerQueue::addTimerInLoop() timer is nullptr");
        return;
    }

    bool earliestChanged = insert(timer);
    if (earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();

    // 检查timerId是否有效
    if (timerId.timer_ == nullptr)
    {
        LOG_ERROR("TimerQueue::cancelInLoop() invalid timerId (timer_ is nullptr)");
        return;
    }

    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);

    if (it != activeTimers_.end())
    {
        // 找到定时器，从两个集合中删除
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        if (n != 1)
        {
            LOG_ERROR("TimerQueue::cancelInLoop() failed to erase timer from timers_, n=%zu", n);
        }
        activeTimers_.erase(it);
        delete it->first;
    }
    else if (callingExpiredTimers_)
    {
        // 定时器正在执行中，添加到取消集合
        cancelingTimers_.insert(timer);
    }
    else
    {
        // 定时器不存在且不在执行中，可能是已经被取消或已过期
        LOG_DEBUG("TimerQueue::cancelInLoop() timer not found, sequence=%ld", timerId.sequence_);
    }
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (const Entry& it : expired)
    {
        try
        {
            it.second->run();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("TimerQueue::handleRead() timer callback exception: %s", e.what());
        }
        catch (...)
        {
            LOG_ERROR("TimerQueue::handleRead() timer callback unknown exception");
        }
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;

    // 检查时间戳是否有效
    if (!now.valid())
    {
        LOG_ERROR("TimerQueue::getExpired() invalid timestamp");
        return expired;
    }

    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);

    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry& it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        if (n != 1)
        {
            LOG_ERROR("TimerQueue::getExpired() failed to erase timer from activeTimers_, n=%zu", n);
        }
    }

    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    // 检查时间戳是否有效
    if (!now.valid())
    {
        LOG_ERROR("TimerQueue::reset() invalid timestamp");
        return;
    }

    for (const Entry& it : expired)
    {
        // 检查timer是否有效
        if (it.second == nullptr)
        {
            LOG_ERROR("TimerQueue::reset() timer is nullptr");
            continue;
        }

        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat() &&
            cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.second->restart(now);
            if (!insert(it.second))
            {
                LOG_ERROR("TimerQueue::reset() failed to reinsert repeating timer");
            }
        }
        else
        {
            delete it.second;
        }
    }

    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer* timer)
{
    loop_->assertInLoopThread();

    // 检查timer是否有效
    if (timer == nullptr)
    {
        LOG_ERROR("TimerQueue::insert() timer is nullptr");
        return false;
    }

    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator, bool> result
            = timers_.insert(Entry(when, timer));
        if (!result.second)
        {
            LOG_ERROR("TimerQueue::insert() failed to insert timer into timers_");
            return false;
        }
    }

    {
        std::pair<ActiveTimerSet::iterator, bool> result
            = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        if (!result.second)
        {
            LOG_ERROR("TimerQueue::insert() failed to insert timer into activeTimers_");
            // 从timers_中移除已插入的定时器
            timers_.erase(Entry(when, timer));
            return false;
        }
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
