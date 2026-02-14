#include "Acceptor.h"
#include "Eventloop.h"
#include "InetAddress.h"
#include "Logger.h"
#include <cassert>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <memory>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <atomic>

// 测试新连接的回调函数
std::atomic<int> newConnectionCount{0};
int lastConnfd = -1;
std::unique_ptr<InetAddress> lastPeerAddr;

// 连接信息结构体
struct ConnectionInfo {
    int connfd;
    InetAddress peerAddr;
};

std::vector<ConnectionInfo> connectionInfos;
std::mutex connectionMutex;

void onNewConnection(int connfd, const InetAddress& peerAddr) {
    newConnectionCount++;
    lastConnfd = connfd;
    lastPeerAddr = std::make_unique<InetAddress>(peerAddr);

    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        connectionInfos.push_back({connfd, peerAddr});
    }
    std::cout << "New connection from: " << peerAddr.toIpPort() 
              << ", connfd: " << connfd << std::endl;
}

void test_acceptor_creation() {
    std::cout << "=== Test Acceptor Creation ===" << std::endl;

    EventLoop loop;
    InetAddress listenAddr(18084, "127.0.0.1");

    // 测试 reuseport 为 true 的情况
    Acceptor acceptor1(&loop, listenAddr, true);
    assert(!acceptor1.listenning());

    // 测试 reuseport 为 false 的情况
    InetAddress listenAddr2(18085, "127.0.0.1");
    Acceptor acceptor2(&loop, listenAddr2, false);
    assert(!acceptor2.listenning());

    std::cout << "Acceptor creation test passed" << std::endl;
    std::cout << std::endl;
}

void test_acceptor_listen() {
    std::cout << "=== Test Acceptor Listen ===" << std::endl;

    EventLoop* loop = nullptr;
    Acceptor* acceptor = nullptr;
    std::mutex loopMutex;
    std::condition_variable loopCV;
    bool loopReady = false;

    // 在IO线程中创建EventLoop和Acceptor
    std::thread loopThread([&]() {
        EventLoop localLoop;
        loop = &localLoop;

        InetAddress listenAddr(18086, "127.0.0.1");
        Acceptor localAcceptor(&localLoop, listenAddr, true);
        acceptor = &localAcceptor;

        acceptor->setNewConnectionCallback(onNewConnection);
        acceptor->listen();

        assert(acceptor->listenning());

        // 通知主线程事件循环已就绪
        {
            std::lock_guard<std::mutex> lock(loopMutex);
            loopReady = true;
        }
        loopCV.notify_one();

        // 运行事件循环
        localLoop.loop();
    });

    // 等待事件循环启动
    {
        std::unique_lock<std::mutex> lock(loopMutex);
        loopCV.wait(lock, [&]{ return loopReady; });
    }

    // 创建客户端连接
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(18086);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    int ret = connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);
    assert(ret == 0);

    // 等待连接被接受（最多等待5秒）
    int waitCount = 0;
    while (newConnectionCount == 0 && waitCount < 50) {
        usleep(100000); // 100ms
        waitCount++;
    }

    // 验证连接是否被接受
    assert(newConnectionCount == 1);
    assert(lastConnfd >= 0);
    assert(lastPeerAddr != nullptr);

    std::cout << "Accepted connection from: " << lastPeerAddr->toIpPort() << std::endl;

    // 清理
    close(lastConnfd);
    close(clientfd);

    // 退出事件循环
    loop->quit();
    loopThread.join();

    std::cout << "Acceptor listen test passed" << std::endl;
    std::cout << std::endl;
}

void test_acceptor_multiple_connections() {
    std::cout << "=== Test Acceptor Multiple Connections ===" << std::endl;

    // 重置计数器和连接信息
    newConnectionCount = 0;
    lastConnfd = -1;
    lastPeerAddr.reset();
    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        connectionInfos.clear();
    }

    std::mutex loopMutex;
    std::condition_variable loopCV;
    bool loopReady = false;
    EventLoop* loop = nullptr;

    // 在IO线程中创建EventLoop和Acceptor
    std::thread loopThread([&]() {
        EventLoop localLoop;
        loop = &localLoop;

        InetAddress listenAddr(18087, "127.0.0.1");
        Acceptor acceptor(&localLoop, listenAddr, true);

        acceptor.setNewConnectionCallback(onNewConnection);
        acceptor.listen();

        // 通知主线程事件循环已就绪
        {
            std::lock_guard<std::mutex> lock(loopMutex);
            loopReady = true;
        }
        loopCV.notify_one();

        // 运行事件循环
        localLoop.loop();
    });

    // 等待事件循环启动
    {
        std::unique_lock<std::mutex> lock(loopMutex);
        loopCV.wait(lock, [&]{ return loopReady; });
    }

    // 创建多个客户端连接
    const int numConnections = 3;
    std::vector<int> clientfds;

    for (int i = 0; i < numConnections; ++i) {
        int clientfd = socket(AF_INET, SOCK_STREAM, 0);
        if (clientfd < 0) {
            // 清理已创建的socket
            for (int fd : clientfds) {
                close(fd);
            }
            assert(false && "Failed to create socket");
        }
        clientfds.push_back(clientfd);

        sockaddr_in serverAddr;
        bzero(&serverAddr, sizeof serverAddr);
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(18087);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        int ret = connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);
        if (ret != 0) {
            // 清理已创建的socket
            for (int fd : clientfds) {
                close(fd);
            }
            assert(false && "Failed to connect");
        }

        // 短暂等待，避免连接过快
        usleep(10000); // 10ms
    }

    // 等待所有连接被接受（最多等待5秒）
    int waitCount = 0;
    while (newConnectionCount < numConnections && waitCount < 50) {
        usleep(100000); // 100ms
        waitCount++;
    }

    // 验证所有连接是否被接受
    assert(newConnectionCount == numConnections);

    // 验证连接信息
    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        assert(connectionInfos.size() == numConnections);

        std::cout << "Total connections accepted: " << newConnectionCount << std::endl;
        for (size_t i = 0; i < connectionInfos.size(); ++i) {
            const auto& info = connectionInfos[i];
            std::cout << "Connection " << i << ": fd=" << info.connfd 
                      << ", peer=" << info.peerAddr.toIpPort() << std::endl;
            assert(info.connfd >= 0);
            close(info.connfd);
        }
    }

    // 清理客户端socket
    for (int fd : clientfds) {
        close(fd);
    }

    // 退出事件循环
    loop->quit();
    loopThread.join();

    std::cout << "Acceptor multiple connections test passed" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Acceptor Tests ===" << std::endl;
    std::cout << std::endl;

    test_acceptor_creation();
    test_acceptor_listen();
    test_acceptor_multiple_connections();

    // 清理
    lastPeerAddr.reset();

    std::cout << "=== All Acceptor Tests Passed ===" << std::endl;
    return 0;
}
