#include "TcpServer.h"
#include "TcpConnection.h"
#include "Eventloop.h"
#include "InetAddress.h"
#include "Timestamp.h"
#include "Buffer.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// 测试连接超时功能
void test_connection_timeout() {
    cout << "=== Test Connection Timeout ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9991);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    // 设置2秒连接超时
    server.setConnectionTimeout(2.0);

    atomic<int> connectionCount(0);
    atomic<int> disconnectionCount(0);

    server.setConnectionCallback([&connectionCount, &disconnectionCount](const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            connectionCount++;
            cout << "Connection established, total: " << connectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        } else {
            disconnectionCount++;
            cout << "Connection closed, total disconnected: " << disconnectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        }
    });

    server.start();

    // 在另一个线程中创建客户端连接
    thread clientThread([&]() {
        this_thread::sleep_for(chrono::milliseconds(100));

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            return;
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9991);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("connect");
            close(sockfd);
            return;
        }

        cout << "Client connected" << endl;

        // 保持连接，等待服务器超时关闭
        // 客户端不主动关闭，等待服务器因为超时而关闭连接
        this_thread::sleep_for(chrono::seconds(5));

        // 此时连接应该已经被服务器关闭
        close(sockfd);
    });

    // 运行5秒后退出，给足够时间让超时触发
    thread t([&loop]() {
        this_thread::sleep_for(chrono::seconds(5));
        loop.quit();
    });
    t.detach();

    loop.loop();
    clientThread.join();

    assert(connectionCount == 1);
    assert(disconnectionCount == 1);

    cout << "Connection timeout test passed" << endl;
}

// 测试空闲超时功能
void test_idle_timeout() {
    cout << "=== Test Idle Timeout ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9992);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    // 设置2秒空闲超时
    server.setIdleTimeout(2.0);

    atomic<int> connectionCount(0);
    atomic<int> disconnectionCount(0);
    atomic<int> messageCount(0);

    server.setConnectionCallback([&connectionCount, &disconnectionCount](const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            connectionCount++;
            cout << "Connection established, total: " << connectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        } else {
            disconnectionCount++;
            cout << "Connection closed, total disconnected: " << disconnectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        }
    });

    server.setMessageCallback([&messageCount](const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        std::string msg = buf->retrieveAllAsString();
        messageCount++;
        cout << "Received message: " << msg << ", total: " << messageCount << endl;
        // 回显消息
        conn->send(msg);
    });

    server.start();

    // 在另一个线程中创建客户端连接
    thread clientThread([&]() {
        this_thread::sleep_for(chrono::milliseconds(100));

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            return;
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9992);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("connect");
            close(sockfd);
            return;
        }

        cout << "Client connected" << endl;

        // 发送一条消息
        const char* msg = "Hello";
        send(sockfd, msg, strlen(msg), 0);
        cout << "Client sent message: " << msg << endl;

        // 等待接收回显
        char buf[1024];
        int n = recv(sockfd, buf, sizeof(buf), 0);
        if (n > 0) {
            buf[n] = '\0';
            cout << "Client received echo: " << buf << ", length: " << n << endl;
        }

        // 保持连接但不发送消息，等待空闲超时
        // 空闲超时设置为2秒，这里等待5秒确保超时触发
        this_thread::sleep_for(chrono::seconds(5));

        // 此时连接应该已经被服务器因为空闲超时而关闭
        close(sockfd);
    });

    // 运行5秒后退出，给足够时间让超时触发
    thread t([&loop]() {
        this_thread::sleep_for(chrono::seconds(5));
        loop.quit();
    });
    t.detach();

    loop.loop();
    clientThread.join();

    assert(connectionCount == 1);
    assert(disconnectionCount == 1);
    assert(messageCount == 1);

    cout << "Idle timeout test passed" << endl;
}

// 测试心跳功能
void test_keepalive() {
    cout << "=== Test KeepAlive ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9993);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    // 启用心跳，间隔1秒
    server.setKeepAlive(true, 1);

    atomic<int> connectionCount(0);
    atomic<int> disconnectionCount(0);
    atomic<int> heartbeatCount(0);
    atomic<int> messageCount(0);

    server.setConnectionCallback([&connectionCount, &disconnectionCount](const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            connectionCount++;
            cout << "Connection established, total: " << connectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        } else {
            disconnectionCount++;
            cout << "Connection closed, total disconnected: " << disconnectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        }
    });

    server.setMessageCallback([&messageCount, &heartbeatCount](const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        std::string msg = buf->retrieveAllAsString();
        messageCount++;
        cout << "Received message: " << msg << ", total: " << messageCount << endl;
        
        // 检查是否是心跳包
        if (msg == "HEARTBEAT") {
            heartbeatCount++;
            cout << "Received heartbeat, total: " << heartbeatCount << endl;
        }
        
        // 回显消息
        conn->send(msg);
    });

    server.start();

    // 在另一个线程中创建客户端连接
    thread clientThread([&]() {
        this_thread::sleep_for(chrono::milliseconds(100));

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            return;
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9993);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("connect");
            close(sockfd);
            return;
        }

        cout << "Client connected" << endl;

        // 发送一些消息给服务器
        for (int i = 0; i < 3; ++i) {
            const char* msg = "Hello";
            send(sockfd, msg, strlen(msg), 0);
            cout << "Client sent message " << i + 1 << ": " << msg << endl;
            
            // 接收回显
            char buf[1024];
            int n = recv(sockfd, buf, sizeof(buf), 0);
            if (n > 0) {
                buf[n] = '\0';
                cout << "Client received echo: " << buf << ", length: " << n << endl;
            }
            
            this_thread::sleep_for(chrono::seconds(1));
        }
        
        close(sockfd);
        cout << "Client closed connection" << endl;
        
        // 等待服务器端处理连接关闭事件
        this_thread::sleep_for(chrono::milliseconds(500));
    });

    // 运行5秒后退出，给服务器端足够的时间来处理客户端关闭连接的事件
    thread t([&loop]() {
        this_thread::sleep_for(chrono::seconds(5));
        loop.quit();
    });
    t.detach();

    loop.loop();
    clientThread.join();

    assert(connectionCount == 1);
    assert(disconnectionCount == 1);
    // 应该收到3条消息
    assert(messageCount == 3);
    // 心跳功能已启用，但不应该收到心跳包，因为心跳包是服务器发送给客户端的
    assert(heartbeatCount == 0);

    cout << "KeepAlive test passed" << endl;
}

// 测试组合功能
void test_combined_features() {
    cout << "=== Test Combined Features ===" << endl;

    EventLoop loop;
    InetAddress listenAddr(9994);
    TcpServer server(&loop, listenAddr, TcpServer::kReusePort);

    // 设置组合超时
    server.setConnectionTimeout(5.0);  // 5秒连接超时
    server.setIdleTimeout(3.0);        // 3秒空闲超时
    server.setKeepAlive(true, 1);       // 1秒心跳

    atomic<int> connectionCount(0);
    atomic<int> disconnectionCount(0);
    atomic<int> messageCount(0);

    server.setConnectionCallback([&connectionCount, &disconnectionCount](const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            connectionCount++;
            cout << "Connection established, total: " << connectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        } else {
            disconnectionCount++;
            cout << "Connection closed, total disconnected: " << disconnectionCount << ", peer: " << conn->peerAddress().toIpPort() << endl;
        }
    });

    server.setMessageCallback([&messageCount](const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        std::string msg = buf->retrieveAllAsString();
        messageCount++;
        cout << "Received message: " << msg << ", total: " << messageCount << endl;
        // 回显消息
        conn->send(msg);
    });

    server.start();

    // 在另一个线程中创建客户端连接
    thread clientThread([&]() {
        this_thread::sleep_for(chrono::milliseconds(100));

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            return;
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(9994);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("connect");
            close(sockfd);
            return;
        }

        cout << "Client connected" << endl;

        // 定期发送消息以保持连接活跃
        for (int i = 0; i < 5; i++) {
            this_thread::sleep_for(chrono::seconds(1));
            const char* msg = "Keep alive message";
            send(sockfd, msg, strlen(msg), 0);
            cout << "Client sent message " << i + 1 << ": " << msg << endl;

            // 接收回显
            char buf[1024];
            int n = recv(sockfd, buf, sizeof(buf), 0);
            if (n > 0) {
                buf[n] = '\0';
                cout << "Client received echo: " << buf << ", length: " << n << endl;
            } else {
                cout << "Client failed to receive echo, connection may be closed" << endl;
                break;
            }
        }

        // 停止发送消息，等待空闲超时
        cout << "Client stopping messages, waiting for idle timeout..." << endl;
        this_thread::sleep_for(chrono::seconds(4));

        close(sockfd);
    });

    // 运行8秒后退出
    thread t([&loop]() {
        this_thread::sleep_for(chrono::seconds(8));
        loop.quit();
    });
    t.detach();

    loop.loop();
    clientThread.join();

    assert(connectionCount == 1);
    assert(disconnectionCount == 1);
    // 由于连接可能在发送第5条消息前被关闭，我们检查messageCount是否在合理范围内
    cout << "Final message count: " << messageCount << endl;
    assert(messageCount >= 3 && messageCount <= 5);

    cout << "Combined features test passed" << endl;
}

int main() {
    cout << "=== TCP Timer Integration Tests ===" << endl;

    test_connection_timeout();
    test_idle_timeout();
    test_keepalive();
    test_combined_features();

    cout << "=== All TCP Timer Integration Tests Passed ===" << endl;
    return 0;
}
