
#include "Eventloop.h"
#include "Channel.h"
#include "Logger.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <unistd.h>
#include <sys/eventfd.h>

void test_eventloop_basic() {
    std::cout << "Test EventLoop basic functionality" << std::endl;

    EventLoop loop;
    assert(loop.isInLoopThread());
    std::cout << "EventLoop is in the correct thread" << std::endl;

    // 在另一个线程中调用quit，测试跨线程退出
    std::thread quitThread([&loop]() {
        sleep(1);  // 等待1秒
        loop.quit();
    });

    // 运行事件循环
    loop.loop();

    quitThread.join();

    std::cout << "EventLoop basic test passed" << std::endl;
}

void test_eventloop_channel() {
    std::cout << "Test EventLoop with Channel" << std::endl;

    // 使用指针来在多线程间共享 EventLoop
    EventLoop* loopPtr = nullptr;
    int eventfd = -1;
    bool callbackCalled = false;

    // 在主线程中创建eventfd，确保生命周期正确
    eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(eventfd >= 0);

    // 在新线程中创建 EventLoop 并运行 loop()
    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr = &loop;
        
        // eventfd已在主线程中创建
        
        // 创建Channel并设置读回调
        Channel channel(&loop, eventfd);
        channel.setReadCallback([&callbackCalled](Timestamp receiveTime) {
            callbackCalled = true;
            std::cout << "Read callback called at " << receiveTime.to_string() << std::endl;
        });
        
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
    std::thread writerThread([eventfd]() {
        sleep(1);  // 等待1秒
        uint64_t one = 1;
        ssize_t n = ::write(eventfd, &one, sizeof(one));
        if (n != sizeof(one))
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // eventfd计数器已经非零，这是正常情况
                std::cout << "eventfd counter already non-zero" << std::endl;
            }
            else
            {
                // 其他错误
                std::cerr << "write failed, errno=" << errno << std::endl;
            }
        }
    });

    // 等待2秒后退出事件循环
    sleep(2);
    
    // 使用 runInLoop 在事件循环线程中调用 quit()
    loopPtr->runInLoop([loopPtr]() {
        loopPtr->quit();
    });

    writerThread.join();
    loopThread.join();

    // 所有线程都不再使用eventfd，现在可以安全关闭
    close(eventfd);

    assert(callbackCalled);

    std::cout << "EventLoop channel test passed" << std::endl;
}

void test_eventloop_runinloop() {
    std::cout << "Test EventLoop::runInLoop" << std::endl;

    // 使用指针来在多线程间共享 EventLoop
    EventLoop* loopPtr = nullptr;
    bool inLoopCallbackCalled = false;
    bool queuedCallbackCalled = false;

    // 在新线程中创建 EventLoop 并运行 loop()
    std::thread loopThread([&]() {
        EventLoop loop;
        loopPtr = &loop;
        
        // 运行事件循环
        loop.loop();
    });

    // 等待一小段时间，确保事件循环线程已经启动
    sleep(0.1);

    // 在IO线程中调用runInLoop
    loopPtr->runInLoop([&inLoopCallbackCalled]() {
        inLoopCallbackCalled = true;
        std::cout << "runInLoop callback executed in IO thread" << std::endl;
    });

    // 在另一个线程中调用runInLoop
    std::thread otherThread([loopPtr, &queuedCallbackCalled]() {
        loopPtr->runInLoop([&queuedCallbackCalled]() {
            queuedCallbackCalled = true;
            std::cout << "runInLoop callback queued to IO thread" << std::endl;
        });
    });

    // 等待1秒后退出事件循环
    sleep(1);
    
    // 使用 runInLoop 在事件循环线程中调用 quit()
    loopPtr->runInLoop([loopPtr]() {
        loopPtr->quit();
    });

    otherThread.join();
    loopThread.join();

    assert(inLoopCallbackCalled);
    assert(queuedCallbackCalled);

    std::cout << "EventLoop::runInLoop test passed" << std::endl;
}

int main() {
    std::cout << "=== EventLoop Tests ===" << std::endl;
    test_eventloop_basic();
    test_eventloop_channel();
    test_eventloop_runinloop();
    std::cout << "=== All EventLoop Tests Passed ===" << std::endl;
    return 0;
}
