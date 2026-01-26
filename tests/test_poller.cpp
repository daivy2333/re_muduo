
#include "Poller.h"
#include "EpollPoller.h"
#include "Eventloop.h"
#include "Channel.h"
#include "Logger.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <unistd.h>
#include <sys/eventfd.h>

void test_poller_creation() {
    std::cout << "Test Poller creation" << std::endl;

    EventLoop loop;
    Poller* poller = Poller::newDefaultPoller(&loop);
    assert(poller != nullptr);

    delete poller;
    std::cout << "Poller creation test passed" << std::endl;
}

void test_epollpoller_creation() {
    std::cout << "Test EpollPoller creation" << std::endl;

    EventLoop loop;
    EpollPoller epollPoller(&loop);

    std::cout << "EpollPoller creation test passed" << std::endl;
}

void test_epollpoller_channel_operations() {
    std::cout << "Test EpollPoller channel operations" << std::endl;

    // 使用指针来在多线程间共享 EventLoop
    EventLoop* loopPtr = nullptr;
    EpollPoller* pollerPtr = nullptr;
    int fd1 = -1;
    int fd2 = -1;

    // 在新线程中创建 EventLoop 并运行 loop()
    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr = &loop;
        EpollPoller poller(&loop);
        pollerPtr = &poller;

        fd1 = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        fd2 = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(fd1 >= 0 && fd2 >= 0);

        Channel channel1(&loop, fd1);
        Channel channel2(&loop, fd2);

        // 测试添加Channel
        channel1.enableReading();
        assert(loop.hasChannel(&channel1));

        channel2.enableReading();
        assert(loop.hasChannel(&channel2));

        // 测试更新Channel
        channel1.enableWriting();
        assert(loop.hasChannel(&channel1));

        // 测试移除Channel
        channel1.disableAll();
        loop.removeChannel(&channel1);
        assert(!loop.hasChannel(&channel1));

        channel2.disableAll();
        loop.removeChannel(&channel2);
        assert(!loop.hasChannel(&channel2));

        // 清理
        close(fd1);
        close(fd2);
    });

    // 等待线程完成
    loopThread.join();

    std::cout << "EpollPoller channel operations test passed" << std::endl;
}

void test_epollpoller_poll() {
    std::cout << "Test EpollPoller poll" << std::endl;

    // 使用指针来在多线程间共享 EventLoop
    EventLoop* loopPtr = nullptr;
    int fd = -1;
    bool callbackCalled = false;

    // 在新线程中创建 EventLoop 并运行 loop()
    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr = &loop;
        // 获取 EventLoop 内部的 Poller
        Poller* poller = loop.getPoller();

        fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(fd >= 0);

        Channel channel(&loop, fd);
        channel.setReadCallback([&callbackCalled](Timestamp receiveTime) {
            callbackCalled = true;
            std::cout << "Poll read callback called at " << receiveTime.to_string() << std::endl;
        });

        channel.enableReading();

        // 在另一个线程中写入eventfd，触发读事件
        std::thread writerThread([fd]() {
            sleep(1);  // 等待1秒
            uint64_t one = 1;
            ssize_t n = ::write(fd, &one, sizeof(one));
            assert(n == sizeof(one));
        });

        // 调用poll等待事件
        Poller::ChannelList activeChannels;
        Timestamp receiveTime = poller->poll(2000, &activeChannels);

        assert(!activeChannels.empty());
        assert(activeChannels[0] == &channel);

        // 处理活跃的Channel
        for (Channel* ch : activeChannels) {
            ch->handleEvent(receiveTime);
        }

        assert(callbackCalled);

        writerThread.join();
        close(fd);
    });

    // 等待线程完成
    loopThread.join();

    std::cout << "EpollPoller poll test passed" << std::endl;
}

int main() {
    std::cout << "=== Poller Tests ===" << std::endl;
    test_poller_creation();
    test_epollpoller_creation();
    test_epollpoller_channel_operations();
    test_epollpoller_poll();
    std::cout << "=== All Poller Tests Passed ===" << std::endl;
    return 0;
}
