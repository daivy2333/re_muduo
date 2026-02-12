#include "echo_server_bench.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "Timestamp.h"
#include "AsyncLoggingInit.h"
#include "Logger.h"
#include <chrono>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

EchoServerBench::EchoServerBench(int port, int payload_size)
    : port_(port), payload_size_(payload_size),
      loop_(nullptr), server_(nullptr), received_bytes_(0) {}

void EchoServerBench::setup() {
    // 初始化异步日志
    initAsyncLogging("log/echo_bench", 1024 * 1024 * 100, 3);  // 100MB滚动, 3秒刷新
    setAsyncOutput();
    
    LOG_INFO("EchoServerBench setup started, port=%d, payload_size=%d", port_, payload_size_);
    
    // 先创建服务器线程，在该线程中创建EventLoop
    server_thread_.reset(new std::thread([this] {
        loop_ = new EventLoop;  // 在服务器线程中创建EventLoop
        InetAddress listenAddr(port_);
        server_ = new TcpServer(loop_, listenAddr);

        server_->setConnectionCallback([this](const TcpConnectionPtr& conn) {
            if (conn->connected()) {
                LOG_INFO("New connection: %s", conn->name().c_str());
            } else {
                LOG_INFO("Connection closed: %s", conn->name().c_str());
            }
        });

        server_->setMessageCallback([this](const TcpConnectionPtr& conn,
                                          Buffer* buf,
                                          Timestamp receiveTime) {
            received_bytes_.fetch_add(buf->readableBytes());
            conn->send(buf);
        });

        // 【参数调优】服务器线程数：4(默认) -> 8(高负荷)
        // 增加线程数可以提高并发处理能力
        server_->setThreadNum(8);
        LOG_INFO("Server thread count set to 8");
        server_->start();
        LOG_INFO("Server started on port %d", port_);

        loop_->loop();  // 在服务器线程中运行loop()

        // 在服务器线程中销毁server_和loop_
        delete server_;
        server_ = nullptr;
        delete loop_;
        loop_ = nullptr;
    }));

    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

BenchmarkBase::Result EchoServerBench::run(int concurrent_clients, int duration_seconds) {
        std::vector<std::thread> clients;
        std::atomic<int64_t> total_requests{0};
        std::atomic<int64_t> total_bytes{0};
        std::atomic<bool> running{true};

        auto start_time = Timestamp::now();

        // 启动多个客户端
        for (int i = 0; i < concurrent_clients; ++i) {
            clients.emplace_back([this, &total_requests, &total_bytes, &running] {
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) {
                    LOG_ERROR("Failed to create socket: %s", strerror(errno));
                    return;
                }

                struct sockaddr_in serv_addr;
                memset(&serv_addr, 0, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(port_);
                inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

                if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                    LOG_ERROR("Failed to connect to server: %s", strerror(errno));
                    close(sockfd);
                    return;
                }

                // 使用阻塞socket，简化测试逻辑
                // int flags = fcntl(sockfd, F_GETFL, 0);
                // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

                char* payload = new char[payload_size_];
                memset(payload, 'A', payload_size_);

                while (running.load()) {
                    auto send_start = Timestamp::now();

                    // 发送数据
                    ssize_t sent = send(sockfd, payload, payload_size_, 0);
                    if (sent < 0) {
                        LOG_ERROR("Send failed: %s", strerror(errno));
                        break;
                    }

                    // 接收数据
                    char recv_buf[payload_size_];
                    size_t total_received = 0;
                    while (total_received < payload_size_) {
                        ssize_t received = recv(sockfd, recv_buf + total_received,
                                             payload_size_ - total_received, 0);
                        if (received < 0) {
                            LOG_ERROR("Recv failed: %s", strerror(errno));
                            break;
                        }
                        total_received += received;
                    }

                    auto send_end = Timestamp::now();
                    double latency = timeDifference(send_end, send_start) * 1000; // ms
                    recordLatency(latency);

                    total_requests.fetch_add(1);
                    total_bytes.fetch_add(payload_size_ * 2); // 发送+接收
                }

                delete[] payload;
                close(sockfd);
            });
        }

        // 运行指定时间
        std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));
        running.store(false);

        // 停止客户端
        for (auto& client : clients) {
            if (client.joinable()) client.join();
        }

        auto end_time = Timestamp::now();
        double duration = timeDifference(end_time, start_time);

        return calculateResult(total_requests.load(), total_bytes.load(), duration);
    }

void EchoServerBench::teardown() {
    LOG_INFO("EchoServerBench teardown started");
    
    if (loop_) {
        loop_->quit();
        LOG_INFO("EventLoop quit called");
    }
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
        LOG_INFO("Server thread joined");
    }
    
    // 等待日志刷新
    sleep(3);
    LOG_INFO("EchoServerBench teardown completed");
}
