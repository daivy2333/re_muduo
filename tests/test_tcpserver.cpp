#include "TcpServer.h"
#include "Eventloop.h"
#include "InetAddress.h"
#include "Timestamp.h"
#include "TcpConnection.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

// 测试基本的服务器创建和启动
void test_basic_creation() {
    cout << "=== Test Basic Creation ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9981);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    // 设置回调函数
    server.setConnectionCallback([](const TcpConnectionPtr& conn) {
        cout << "Connection " << (conn->connected() ? "established" : "destroyed") << endl;
    });

    server.setMessageCallback([](const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        cout << "Received message" << endl;
    });

    server.start();

    // 运行一段时间后退出
    thread t([&loop]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        loop.quit();
    });
    t.detach();

    loop.loop();

    cout << "Basic creation test passed" << endl;
}

// 测试多线程服务器
void test_multithread() {
    cout << "=== Test Multi-thread Server ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9982);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    // 设置线程数量
    server.setThreadNum(4);

    // 设置回调函数
    server.setConnectionCallback([](const TcpConnectionPtr& conn) {
        cout << "Connection " << (conn->connected() ? "established" : "destroyed") << endl;
    });

    server.setMessageCallback([](const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        cout << "Received message" << endl;
    });

    server.start();

    // 运行一段时间后退出
    thread t([&loop]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        loop.quit();
    });
    t.detach();

    loop.loop();

    cout << "Multi-thread server test passed" << endl;
}

// 测试连接管理
void test_connection_management() {
    cout << "=== Test Connection Management ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9983);
    TcpServer server(&loop, listenAddr);

    int connectionCount = 0;

    server.setConnectionCallback([&connectionCount](const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            connectionCount++;
            cout << "New connection, total: " << connectionCount << endl;
        } else {
            connectionCount--;
            cout << "Connection closed, total: " << connectionCount << endl;
        }
    });

    server.setMessageCallback([](const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        cout << "Received message" << endl;
    });

    server.start();

    // 运行一段时间后退出
    thread t([&loop]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        loop.quit();
    });
    t.detach();

    loop.loop();

    cout << "Connection management test passed" << endl;
}

// 测试线程初始化回调
void test_thread_init_callback() {
    cout << "=== Test Thread Init Callback ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9984);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    server.setThreadNum(2);

    int initCount = 0;
    server.setThreadInitCallback([&initCount](EventLoop* loop) {
        initCount++;
        cout << "Thread initialized" << endl;
    });

    server.setConnectionCallback([](const TcpConnectionPtr& conn) {
        cout << "Connection " << (conn->connected() ? "established" : "destroyed") << endl;
    });

    server.start();

    // 运行一段时间后退出
    thread t([&loop]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        loop.quit();
    });
    t.detach();

    loop.loop();

    assert(initCount == 2); // 应该有2个子线程被初始化
    cout << "Thread init callback test passed" << endl;
}

// 测试服务器级超时设置
void test_server_timeout_settings() {
    cout << "=== Test Server Timeout Settings ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9985);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    // 设置服务器级超时
    server.setConnectionTimeout(10.0);
    server.setIdleTimeout(30.0);
    server.setKeepAlive(true, 15);

    server.setConnectionCallback([](const TcpConnectionPtr& conn) {
        cout << "Connection " << (conn->connected() ? "established" : "destroyed") << endl;
    });

    server.setMessageCallback([](const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        cout << "Received message" << endl;
    });

    server.start();

    // 运行一段时间后退出
    thread t([&loop]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        loop.quit();
    });
    t.detach();

    loop.loop();

    cout << "Server timeout settings test passed" << endl;
}

int main() {
    cout << "=== TcpServer Tests ===" << endl;

    test_basic_creation();
    test_multithread();
    test_connection_management();
    test_thread_init_callback();
    test_server_timeout_settings();

    cout << "=== All TcpServer Tests Passed ===" << endl;
    return 0;
}
