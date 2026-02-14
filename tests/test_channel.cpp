#include "Channel.h"
#include "Eventloop.h"
#include "Logger.h"
#include <cassert>
#include <cerrno>
#include <iostream>
#include <memory>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <assert.h>

// 测试 Channel 的基本创建和属性
void test_channel_creation() {
    std::cout << "Test Channel creation" << std::endl;

    EventLoop loop;
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    Channel channel(&loop, fd);
    assert(channel.fd() == fd);
    assert(channel.isNoneEvent());
    assert(channel.ownerLoop() == &loop);
    assert(channel.index() == -1);  // 初始状态应该是 kNew (-1)
    assert(channel.events() == 0);  // 初始没有关注的事件

    close(fd);
    std::cout << "Channel creation test passed" << std::endl;
}

// 测试 Channel 的事件管理
void test_channel_events() {
    std::cout << "Test Channel events" << std::endl;

    EventLoop loop;
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    Channel channel(&loop, fd);

    // 测试读事件
    channel.enableReading();
    assert(channel.isReading());
    assert(!channel.isWriting());
    assert(channel.events() & (EPOLLIN | EPOLLPRI));

    channel.disableReading();
    assert(!channel.isReading());
    assert(channel.isNoneEvent());

    // 测试写事件
    channel.enableWriting();
    assert(channel.isWriting());
    assert(!channel.isReading());
    assert(channel.events() & EPOLLOUT);

    channel.disableWriting();
    assert(!channel.isWriting());
    assert(channel.isNoneEvent());

    // 测试同时启用读写事件
    channel.enableReading();
    channel.enableWriting();
    assert(channel.isReading());
    assert(channel.isWriting());
    assert(channel.events() & (EPOLLIN | EPOLLPRI | EPOLLOUT));

    // 测试禁用所有事件
    channel.disableAll();
    assert(channel.isNoneEvent());

    close(fd);
    std::cout << "Channel events test passed" << std::endl;
}

// 测试 Channel 的回调函数
void test_channel_callbacks() {
    std::cout << "Test Channel callbacks" << std::endl;

    EventLoop* loopPtr = nullptr;
    int fd = -1;
    bool readCallbackCalled = false;
    bool writeCallbackCalled = false;
    bool closeCallbackCalled = false;
    bool errorCallbackCalled = false;

    fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr = &loop;

        Channel channel(&loop, fd);

        channel.setReadCallback([&readCallbackCalled](Timestamp receiveTime) {
            readCallbackCalled = true;
            std::cout << "Read callback called at " << receiveTime.to_string() << std::endl;
        });

        channel.setWriteCallback([&writeCallbackCalled]() {
            writeCallbackCalled = true;
            std::cout << "Write callback called" << std::endl;
        });

        channel.setCloseCallback([&closeCallbackCalled]() {
            closeCallbackCalled = true;
            std::cout << "Close callback called" << std::endl;
        });

        channel.setErrorCallback([&errorCallbackCalled]() {
            errorCallbackCalled = true;
            std::cout << "Error callback called" << std::endl;
        });

        channel.enableReading();
        loop.loop();

        channel.disableAll();
        channel.remove();
    });

    sleep(0.1);

    std::thread writerThread([fd]() {
        sleep(1);
        uint64_t one = 1;
        ssize_t n = ::write(fd, &one, sizeof(one));
        if (n != sizeof(one)) {
            std::cerr << "Warning: write to eventfd failed, n=" << n << ", errno=" << errno << std::endl;
        }
    });

    sleep(2);

    loopPtr->runInLoop([loopPtr]() {
        loopPtr->quit();
    });

    writerThread.join();
    loopThread.join();

    close(fd);

    assert(readCallbackCalled);
    std::cout << "Channel callbacks test passed" << std::endl;
}

// 测试 tie() 机制
void test_channel_tie() {
    std::cout << "Test Channel tie mechanism" << std::endl;

    EventLoop loop;
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    bool callbackCalled = false;
    std::shared_ptr<int> tiedObj = std::make_shared<int>(42);

    {
        Channel channel(&loop, fd);

        // 设置回调
        channel.setReadCallback([&callbackCalled, tiedObj](Timestamp receiveTime) {
            callbackCalled = true;
            std::cout << "Tied callback called, obj value=" << *tiedObj << std::endl;
        });

        // 绑定对象
        channel.tie(tiedObj);
        channel.enableReading();

        // 触发事件
        uint64_t one = 1;
        ssize_t n = ::write(fd, &one, sizeof(one));
        assert(n == sizeof(one));

        // 手动设置 revents 并处理事件
        channel.setRevents(EPOLLIN);
        channel.handleEvent(Timestamp::now());

        assert(callbackCalled);
    }

    // 测试对象被销毁后的行为
    callbackCalled = false;
    std::weak_ptr<int> weakObj = tiedObj;
    tiedObj.reset();

    {
        Channel channel(&loop, fd);

        channel.setReadCallback([&callbackCalled](Timestamp receiveTime) {
            callbackCalled = true;
            std::cout << "Callback called after object destroyed" << std::endl;
        });

        // 绑定一个即将被销毁的对象
        std::shared_ptr<int> tempObj = std::make_shared<int>(100);
        channel.tie(tempObj);
        tempObj.reset();  // 立即销毁对象

        // 触发事件
        uint64_t one = 1;
        ssize_t n = ::write(fd, &one, sizeof(one));
        assert(n == sizeof(one));

        // 手动设置 revents 并处理事件
        channel.setRevents(EPOLLIN);
        channel.handleEvent(Timestamp::now());

        // 由于对象已被销毁，回调不应该被调用
        assert(!callbackCalled);
    }

    close(fd);
    std::cout << "Channel tie mechanism test passed" << std::endl;
}

// 测试各种事件类型
void test_channel_all_events() {
    std::cout << "Test Channel all event types" << std::endl;

    EventLoop loop;
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    bool hupCallbackCalled = false;
    bool errCallbackCalled = false;
    bool rdhupCallbackCalled = false;
    bool outCallbackCalled = false;

    Channel channel(&loop, fd);

    channel.setCloseCallback([&hupCallbackCalled]() {
        hupCallbackCalled = true;
        std::cout << "HUP callback called" << std::endl;
    });

    channel.setErrorCallback([&errCallbackCalled]() {
        errCallbackCalled = true;
        std::cout << "Error callback called" << std::endl;
    });

    channel.setReadCallback([&rdhupCallbackCalled](Timestamp receiveTime) {
        rdhupCallbackCalled = true;
        std::cout << "RDHUP callback called" << std::endl;
    });

    channel.setWriteCallback([&outCallbackCalled]() {
        outCallbackCalled = true;
        std::cout << "Write callback called" << std::endl;
    });

    // 测试 EPOLLHUP 事件
    channel.setRevents(EPOLLHUP);
    channel.handleEvent(Timestamp::now());
    assert(hupCallbackCalled);

    // 测试 EPOLLERR 事件
    hupCallbackCalled = false;
    channel.setRevents(EPOLLERR);
    channel.handleEvent(Timestamp::now());
    assert(errCallbackCalled);

    // 测试 EPOLLRDHUP 事件
    errCallbackCalled = false;
    channel.setRevents(EPOLLRDHUP);
    channel.handleEvent(Timestamp::now());
    assert(rdhupCallbackCalled);

    // 测试 EPOLLOUT 事件
    rdhupCallbackCalled = false;
    channel.setRevents(EPOLLOUT);
    channel.handleEvent(Timestamp::now());
    assert(outCallbackCalled);

    // 测试多个事件同时触发
    hupCallbackCalled = false;
    errCallbackCalled = false;
    rdhupCallbackCalled = false;
    outCallbackCalled = false;
    channel.setRevents(EPOLLHUP | EPOLLERR | EPOLLRDHUP | EPOLLOUT);
    channel.handleEvent(Timestamp::now());
    assert(hupCallbackCalled);
    assert(errCallbackCalled);
    assert(rdhupCallbackCalled);
    assert(outCallbackCalled);

    close(fd);
    std::cout << "Channel all event types test passed" << std::endl;
}

// 测试边界情况
void test_channel_edge_cases() {
    std::cout << "Test Channel edge cases" << std::endl;

    EventLoop loop;

    // 测试回调函数为空的情况
    {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(fd >= 0);

        Channel channel(&loop, fd);
        // 不设置任何回调
        channel.setRevents(EPOLLIN);
        channel.handleEvent(Timestamp::now());
        // 应该不会崩溃

        close(fd);
    }

    // 测试多次设置 revents
    {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(fd >= 0);

        Channel channel(&loop, fd);
        channel.setRevents(EPOLLIN);
        assert(channel.events() == 0);  // revents 不影响 events
        channel.setRevents(EPOLLOUT);
        channel.setRevents(EPOLLERR);
        // 应该可以正常设置

        close(fd);
    }

    // 测试 index 状态变化
    {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(fd >= 0);

        Channel channel(&loop, fd);
        assert(channel.index() == -1);  // kNew

        channel.setIndex(1);  // kAdded
        assert(channel.index() == 1);

        channel.setIndex(2);  // kDeleted
        assert(channel.index() == 2);

        close(fd);
    }

    // 测试 ownerLoop
    {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(fd >= 0);

        Channel channel(&loop, fd);
        assert(channel.ownerLoop() == &loop);

        close(fd);
    }

    std::cout << "Channel edge cases test passed" << std::endl;
}

// 测试多线程场景
void test_channel_multithread() {
    std::cout << "Test Channel multithread scenarios" << std::endl;

    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    bool callbackCalled = false;
    EventLoop* loopPtr = nullptr;

    std::thread ioThread([&]() {
        EventLoop loop;
        loopPtr = &loop;

        Channel channel(&loop, fd);

        channel.setReadCallback([&callbackCalled](Timestamp receiveTime) {
            callbackCalled = true;
            std::cout << "Read callback in IO thread" << std::endl;
        });

        channel.enableReading();
        loop.loop();

        channel.disableAll();
        channel.remove();
    });

    // 等待 IO 线程启动
    sleep(0.1);

    // 在另一个线程中写入
    std::thread writerThread([fd]() {
        sleep(0.5);
        uint64_t one = 1;
        ssize_t n = ::write(fd, &one, sizeof(one));
        assert(n == sizeof(one));
    });

    // 等待写入完成
    sleep(1);

    // 退出事件循环
    loopPtr->runInLoop([loopPtr]() {
        loopPtr->quit();
    });

    writerThread.join();
    ioThread.join();

    assert(callbackCalled);
    close(fd);
    std::cout << "Channel multithread test passed" << std::endl;
}

// 测试 Channel 的 remove 功能
void test_channel_remove() {
    std::cout << "Test Channel remove" << std::endl;

    EventLoop loop;
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    {
        Channel channel(&loop, fd);
        channel.enableReading();
        channel.remove();
        // remove 后不应该再关注任何事件
    }

    close(fd);
    std::cout << "Channel remove test passed" << std::endl;
}

int main() {
    std::cout << "=== Channel Tests ===" << std::endl;
    test_channel_creation();
    test_channel_events();
    test_channel_callbacks();
    test_channel_tie();
    test_channel_all_events();
    test_channel_edge_cases();
    test_channel_multithread();
    test_channel_remove();
    std::cout << "=== All Channel Tests Passed ===" << std::endl;
    return 0;
}
