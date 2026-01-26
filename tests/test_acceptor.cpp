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

// 测试新连接的回调函数
int newConnectionCount = 0;
int lastConnfd = -1;
InetAddress* lastPeerAddr = nullptr;

void onNewConnection(int connfd, const InetAddress& peerAddr) {
    newConnectionCount++;
    lastConnfd = connfd;
    if (lastPeerAddr == nullptr) {
        lastPeerAddr = new InetAddress(peerAddr);
    } else {
        *lastPeerAddr = peerAddr;
    }
    std::cout << "New connection from: " << peerAddr.toIpPort() 
              << ", connfd: " << connfd << std::endl;
}

void test_acceptor_creation() {
    std::cout << "=== Test Acceptor Creation ===" << std::endl;

    EventLoop loop;
    InetAddress listenAddr(18084, "127.0.0.1");
    Acceptor acceptor(&loop, listenAddr, true);

    assert(!acceptor.listenning());

    std::cout << "Acceptor creation test passed" << std::endl;
    std::cout << std::endl;
}

void test_acceptor_listen() {
    std::cout << "=== Test Acceptor Listen ===" << std::endl;

    EventLoop* loop = nullptr;
    Acceptor* acceptor = nullptr;
    bool testDone = false;

    // 在IO线程中创建EventLoop和Acceptor
    std::thread loopThread([&]() {
        EventLoop localLoop;
        loop = &localLoop;

        InetAddress listenAddr(18085, "127.0.0.1");
        Acceptor localAcceptor(&localLoop, listenAddr, true);
        acceptor = &localAcceptor;

        acceptor->setNewConnectionCallback(onNewConnection);
        acceptor->listen();

        assert(acceptor->listenning());

        // 运行事件循环
        localLoop.loop();

        testDone = true;
    });

    // 给事件循环一点时间启动
    sleep(1);

    // 创建客户端连接
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(18085);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    int ret = connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);
    assert(ret == 0);

    // 等待连接被接受
    sleep(1);

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

    // 重置计数器
    newConnectionCount = 0;
    lastConnfd = -1;
    if (lastPeerAddr) {
        delete lastPeerAddr;
        lastPeerAddr = nullptr;
    }

    EventLoop* loop = nullptr;

    // 在IO线程中创建EventLoop和Acceptor
    std::thread loopThread([&]() {
        EventLoop localLoop;
        loop = &localLoop;

        InetAddress listenAddr(18086, "127.0.0.1");
        Acceptor acceptor(&localLoop, listenAddr, true);

        acceptor.setNewConnectionCallback(onNewConnection);
        acceptor.listen();

        // 运行事件循环
        localLoop.loop();
    });

    // 给事件循环一点时间启动
    sleep(1);

    // 创建多个客户端连接
    const int numConnections = 3;
    int clientfds[numConnections];

    for (int i = 0; i < numConnections; ++i) {
        clientfds[i] = socket(AF_INET, SOCK_STREAM, 0);
        assert(clientfds[i] >= 0);

        sockaddr_in serverAddr;
        bzero(&serverAddr, sizeof serverAddr);
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(18086);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        int ret = connect(clientfds[i], (sockaddr*)&serverAddr, sizeof serverAddr);
        assert(ret == 0);

        // 等待连接被接受
        sleep(1);
    }

    // 验证所有连接是否被接受
    assert(newConnectionCount == numConnections);

    std::cout << "Total connections accepted: " << newConnectionCount << std::endl;

    // 清理
    for (int i = 0; i < numConnections; ++i) {
        close(clientfds[i]);
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
    if (lastPeerAddr) {
        delete lastPeerAddr;
    }

    std::cout << "=== All Acceptor Tests Passed ===" << std::endl;
    return 0;
}
