#include "Poller.h"
#include "EpollPoller.h"
#include <stdlib.h>

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return nullptr; // TODO: 实现 PollPoller
    } else {
        return new EpollPoller(loop);
    }
}