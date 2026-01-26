
#include "Channel.h"
#include "Eventloop.h"
#include "Logger.h"
#include <cassert>
#include <cerrno>
#include <iostream>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>

void test_channel_creation() {
    std::cout << "Test Channel creation" << std::endl;

    EventLoop loop;
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);

    Channel channel(&loop, fd);
    assert(channel.fd() == fd);
    assert(channel.isNoneEvent());

    close(fd);
    std::cout << "Channel creation test passed" << std::endl;
}

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

    channel.disableReading();
    assert(!channel.isReading());
    assert(channel.isNoneEvent());

    // 测试写事件
    channel.enableWriting();
    assert(channel.isWriting());
    assert(!channel.isReading());

    channel.disableWriting();
    assert(!channel.isWriting());
    assert(channel.isNoneEvent());

    // 测试同时启用读写事件
    channel.enableReading();
    channel.enableWriting();
    assert(channel.isReading());
    assert(channel.isWriting());

    // 测试禁用所有事件
    channel.disableAll();
    assert(channel.isNoneEvent());

    close(fd);
    std::cout << "Channel events test passed" << std::endl;
}

void test_channel_callbacks() {
    std::cout << "Test Channel callbacks" << std::endl;

    // 使用指针来在多线程间共享 EventLoop
    EventLoop* loopPtr = nullptr;
    int fd = -1;
    bool readCallbackCalled = false;

    // 在主线程中创建eventfd，确保生命周期正确
    fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(fd >= 0);
    bool writeCallbackCalled = false;
    bool closeCallbackCalled = false;
    bool errorCallbackCalled = false;

    // 在新线程中创建 EventLoop 并运行 loop()
    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr = &loop;
        
        // fd已在主线程中创建
        
        Channel channel(&loop, fd);
        
        // 设置回调函数
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
        
        // 启用读事件
        channel.enableReading();
        
        // 运行事件循环
        loop.loop();
        
        // 清理：先禁用所有事件并从Poller中移除Channel
        channel.disableAll();
        channel.remove();
    });

    // 等待一小段时间，确保事件循环线程已经启动
    sleep(0.1);

    // 在另一个线程中写入eventfd，触发读事件
    std::thread writerThread([fd]() {
        sleep(1);  // 等待1秒
        uint64_t one = 1;
        ssize_t n = ::write(fd, &one, sizeof(one));
        // 非阻塞模式下，如果eventfd已满或发生错误，write可能返回-1
        // 这里我们只记录错误，不中断测试
        if (n != sizeof(one)) {
            std::cerr << "Warning: write to eventfd failed, n=" << n << ", errno=" << errno << std::endl;
        }
    });

    // 等待足够长的时间，确保writerThread完成写入
    sleep(2);
    
    // 使用 runInLoop 在事件循环线程中调用 quit()
    loopPtr->runInLoop([loopPtr]() {
        loopPtr->quit();
    });

    // 等待writerThread完成
    writerThread.join();

    loopThread.join();

    // 所有线程都不再使用fd，现在可以安全关闭
    close(fd);

    assert(readCallbackCalled);
    std::cout << "Channel callbacks test passed" << std::endl;
}

int main() {
    std::cout << "=== Channel Tests ===" << std::endl;
    test_channel_creation();
    test_channel_events();
    test_channel_callbacks();
    std::cout << "=== All Channel Tests Passed ===" << std::endl;
    return 0;
}
