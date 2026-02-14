#include "Timer.h"

std::atomic<int64_t> Timer::s_numCreated_(0);

void Timer::restart(Timestamp now)
{
    if (repeat_)
    {
        // 检查时间戳是否有效
        if (!now.valid())
        {
            expiration_ = Timestamp::invalid();
            return;
        }
        expiration_ = addTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}
