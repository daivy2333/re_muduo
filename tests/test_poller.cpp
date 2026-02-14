
#include "Poller.h"
#include "EpollPoller.h"
#include "Eventloop.h"
#include "EventLoopThread.h"
#include "Channel.h"
#include "Logger.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <errno.h>
#include <vector>
#include <assert.h>
#include <atomic>
#include <chrono>
#include <future>

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

        // 测试初始状态
        assert(channel1.index() == -1);  // kNew
        assert(channel2.index() == -1);  // kNew

        // 测试添加Channel (kNew -> kAdded)
        channel1.enableReading();
        assert(loop.hasChannel(&channel1));
        assert(channel1.index() == 1);  // kAdded

        channel2.enableReading();
        assert(loop.hasChannel(&channel2));
        assert(channel2.index() == 1);  // kAdded

        // 测试更新Channel (kAdded -> kAdded)
        channel1.enableWriting();
        assert(loop.hasChannel(&channel1));
        assert(channel1.index() == 1);  // kAdded

        // 测试禁用事件 (kAdded -> kDeleted)
        channel1.disableAll();
        assert(channel1.index() == 2);  // kDeleted
        assert(loop.hasChannel(&channel1));  // 仍在 channels_ 中

        // 测试重新启用事件 (kDeleted -> kAdded)
        channel1.enableReading();
        assert(channel1.index() == 1);  // kAdded
        assert(loop.hasChannel(&channel1));

        // 测试移除Channel
        channel1.disableAll();
        loop.removeChannel(&channel1);
        assert(!loop.hasChannel(&channel1));
        assert(channel1.index() == -1);  // kNew

        channel2.disableAll();
        loop.removeChannel(&channel2);
        assert(!loop.hasChannel(&channel2));
        assert(channel2.index() == -1);  // kNew

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

// 测试边界条件
void test_epollpoller_edge_cases() {
    std::cout << "Test EpollPoller edge cases" << std::endl;

    // 测试 poll 超时为 0 的情况
    {
        EventLoop* loopPtr = nullptr;

        std::thread ioThread([&]() {
            EventLoop loop;
            loopPtr = &loop;
            Poller* poller = loop.getPoller();

            Poller::ChannelList activeChannels;
            Timestamp receiveTime = poller->poll(0, &activeChannels);
            assert(activeChannels.empty());
            assert(receiveTime.valid());
        });

        ioThread.join();
    }

    // 测试 poll 超时为 -1 的情况（无限等待）
    {
        EventLoop* loopPtr = nullptr;

        std::thread ioThread([&]() {
            EventLoop loop;
            loopPtr = &loop;
            Poller* poller = loop.getPoller();

            // 在另一个线程中唤醒 EventLoop
            std::thread wakeupThread([&loop]() {
                sleep(1);
                loop.wakeup();
            });

            Poller::ChannelList activeChannels;
            // 注意：这里不能使用 -1，否则会阻塞，我们使用一个较大的值
            Timestamp receiveTime = poller->poll(2000, &activeChannels);
            assert(receiveTime.valid());

            wakeupThread.join();
        });

        ioThread.join();
    }

    // 测试大量 Channel 的添加和删除
    {
        EventLoop* loopPtr = nullptr;

        std::thread ioThread([&]() {
            EventLoop loop;
            loopPtr = &loop;

            const int kNumChannels = 100;
            std::vector<int> fds;
            std::vector<Channel*> channels;

            for (int i = 0; i < kNumChannels; ++i) {
                int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
                assert(fd >= 0);
                fds.push_back(fd);

                Channel* channel = new Channel(&loop, fd);
                channel->enableReading();
                channels.push_back(channel);

                assert(loop.hasChannel(channel));
            }

            // 验证所有 Channel 都已添加
            for (Channel* channel : channels) {
                assert(loop.hasChannel(channel));
            }

            // 删除所有 Channel
            for (Channel* channel : channels) {
                channel->disableAll();
                loop.removeChannel(channel);
                assert(!loop.hasChannel(channel));
                delete channel;
            }

            // 关闭所有文件描述符
            for (int fd : fds) {
                close(fd);
            }
        });

        ioThread.join();
    }

    std::cout << "EpollPoller edge cases test passed" << std::endl;
}

// 测试特殊事件类型
void test_epollpoller_special_events() {
    std::cout << "Test EpollPoller special events" << std::endl;

    EventLoop* loopPtr = nullptr;

    std::thread ioThread([&]() {
        EventLoop loop;
        loopPtr = &loop;
        Poller* poller = loop.getPoller();

        // 创建 socket 对
        int sockfd[2];
        assert(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockfd) == 0);

        bool hupCallbackCalled = false;
        bool errCallbackCalled = false;
        bool rdhupCallbackCalled = false;
        bool outCallbackCalled = false;
        bool readCallbackCalled = false;

        Channel* channel = new Channel(&loop, sockfd[0]);

        channel->setCloseCallback([&hupCallbackCalled]() {
            hupCallbackCalled = true;
            std::cout << "HUP callback called" << std::endl;
        });

        channel->setErrorCallback([&errCallbackCalled]() {
            errCallbackCalled = true;
            std::cout << "Error callback called" << std::endl;
        });

        channel->setReadCallback([&rdhupCallbackCalled, &readCallbackCalled](Timestamp receiveTime) {
            rdhupCallbackCalled = true;
            readCallbackCalled = true;
            std::cout << "Read callback called" << std::endl;
        });

        channel->setWriteCallback([&outCallbackCalled]() {
            outCallbackCalled = true;
            std::cout << "Write callback called" << std::endl;
        });

        // 启用读写事件
        channel->enableReading();
        channel->enableWriting();

        // 在另一个线程中关闭对端
        std::thread closeThread([sockfd]() {
            sleep(1);
            close(sockfd[1]);
        });

        // 调用 poll 等待事件
        Poller::ChannelList activeChannels;
        Timestamp receiveTime = poller->poll(2000, &activeChannels);

        // 处理活跃的 Channel
        for (Channel* ch : activeChannels) {
            ch->handleEvent(receiveTime);
        }

        closeThread.join();

        // 关闭 socket
        channel->disableAll();
        loop.removeChannel(channel);
        delete channel;
        close(sockfd[0]);
    });

    ioThread.join();

    std::cout << "EpollPoller special events test passed" << std::endl;
}

// 测试资源管理
void test_epollpoller_resource_management() {
    std::cout << "Test EpollPoller resource management" << std::endl;

    // 测试 Channel 对象生命周期管理
    {
        EventLoop* loopPtr = nullptr;
        int fd = -1;

        std::thread ioThread([&]() {
            EventLoop loop;
            loopPtr = &loop;

            fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            assert(fd >= 0);

            Channel* channel = new Channel(&loop, fd);
            channel->enableReading();
            assert(loop.hasChannel(channel));

            // 删除 Channel 对象
            delete channel;

            // 关闭文件描述符
            close(fd);
        });

        ioThread.join();
    }

    // 测试文件描述符关闭后的行为
    {
        EventLoop* loopPtr = nullptr;
        int fd = -1;

        std::thread ioThread([&]() {
            EventLoop loop;
            loopPtr = &loop;
            Poller* poller = loop.getPoller();

            fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            assert(fd >= 0);

            Channel* channel = new Channel(&loop, fd);
            channel->enableReading();
            assert(loop.hasChannel(channel));

            // 关闭文件描述符
            close(fd);

            // 尝试 poll
            Poller::ChannelList activeChannels;
            Timestamp receiveTime = poller->poll(100, &activeChannels);
            // 应该不会崩溃

            // 移除 Channel
            channel->disableAll();
            loop.removeChannel(channel);
            delete channel;
        });

        ioThread.join();
    }

    // 测试 EpollPoller 对象析构时仍有 Channel 注册的情况
    {
        EventLoop* loopPtr = nullptr;
        int fd = -1;

        std::thread ioThread([&]() {
            EventLoop loop;
            loopPtr = &loop;

            EpollPoller* poller = new EpollPoller(&loop);

            fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            assert(fd >= 0);

            Channel* channel = new Channel(&loop, fd);
            channel->enableReading();
            assert(loop.hasChannel(channel));

            // 删除 EpollPoller 对象
            delete poller;

            // 清理
            delete channel;
            close(fd);
        });

        ioThread.join();
    }

    std::cout << "EpollPoller resource management test passed" << std::endl;
}

// 测试并发场景
void test_epollpoller_concurrent() {
    std::cout << "Test EpollPoller concurrent scenarios" << std::endl;

    // 使用EventLoopThread来创建和管理EventLoop，符合muduo设计原则
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();

    const int kNumChannels = 10;
    std::vector<int> fds;
    std::vector<Channel*> channels;
    std::atomic<int> added_count{0};
    std::atomic<int> updated_count{0};
    std::atomic<int> removed_count{0};

    // 在主线程中创建 Channel 和文件描述符
    for (int i = 0; i < kNumChannels; ++i) {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(fd >= 0);
        fds.push_back(fd);

        Channel* channel = new Channel(loop, fd);
        channels.push_back(channel);
    }

    // 步骤1：多线程添加 Channel（通过runInLoop）
    {
        std::vector<std::thread> addThreads;
        for (int i = 0; i < kNumChannels; ++i) {
            addThreads.emplace_back([i, &channels, loop, &added_count]() {
                loop->runInLoop([i, &channels, &added_count]() {
                    channels[i]->enableReading();
                    added_count++;
                });
            });
        }

        for (auto& thread : addThreads) {
            thread.join();
        }

        // 等待所有添加操作完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 步骤2：多线程更新 Channel（通过runInLoop）
    {
        std::vector<std::thread> updateThreads;
        for (int i = 0; i < kNumChannels; ++i) {
            updateThreads.emplace_back([i, &channels, loop, &updated_count]() {
                loop->runInLoop([i, &channels, &updated_count]() {
                    channels[i]->enableWriting();
                    updated_count++;
                });
            });
        }

        for (auto& thread : updateThreads) {
            thread.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 步骤3：多线程移除 Channel（通过runInLoop）
    {
        std::vector<std::thread> removeThreads;
        for (int i = 0; i < kNumChannels; ++i) {
            removeThreads.emplace_back([i, &channels, loop, &removed_count]() {
                loop->runInLoop([i, &channels, loop, &removed_count]() {
                    channels[i]->disableAll();
                    loop->removeChannel(channels[i]);
                    removed_count++;
                });
            });
        }

        for (auto& thread : removeThreads) {
            thread.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // 验证所有操作都已完成
    assert(added_count == kNumChannels);
    assert(updated_count == kNumChannels);
    assert(removed_count == kNumChannels);

    // 步骤4：清理资源（在IO线程中执行）
    loop->runInLoop([&channels, &fds]() {
        for (Channel* channel : channels) {
            delete channel;
        }
        channels.clear();

        for (int fd : fds) {
            close(fd);
        }
        fds.clear();
    });

    // 等待清理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "EpollPoller concurrent scenarios test passed" << std::endl;
}

int main() {
    std::cout << "=== Poller Tests ===" << std::endl;
    test_poller_creation();
    test_epollpoller_creation();
    test_epollpoller_channel_operations();
    test_epollpoller_poll();
    test_epollpoller_edge_cases();
    test_epollpoller_special_events();
    test_epollpoller_resource_management();
    test_epollpoller_concurrent();
    std::cout << "=== All Poller Tests Passed ===" << std::endl;
    return 0;
}
