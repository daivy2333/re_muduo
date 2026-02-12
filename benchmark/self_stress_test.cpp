#include "TcpServer.h"
#include "TcpConnection.h"
#include "Eventloop.h"
#include "Buffer.h"
#include "Timestamp.h"
#include "InetAddress.h"
#include "Timer.h"
#include "Logger.h"
#include "AsyncLoggingInit.h"
#include <chrono>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <fstream>
#include <iomanip>

class SelfStressTest {
public:
    SelfStressTest(int port, int clients, int duration)
        : port_(port), clients_(clients), duration_(duration),
          server_loop_(nullptr), server_(nullptr),
          total_connections_(0), active_connections_(0) {}

    void run() {
        // 初始化异步日志
        initAsyncLogging("log/self_stress_test", 1024 * 1024 * 100, 3);  // 100MB滚动, 3秒刷新
        setAsyncOutput();
        
        // 设置日志级别为INFO，记录更多详细信息
        Logger::instance().setLogLevel(INFO);
        
        LOG_INFO("=== Self Stress Test Started ===");
        LOG_INFO("Port: %d, Clients: %d, Duration: %ds", port_, clients_, duration_);

        // 创建输出文件
        std::string report_path = "/home/daivy/projects/muduo_learn/re_muduo/self_stress_report.txt";
        report_.open(report_path);
        if (!report_.is_open()) {
            LOG_ERROR("Failed to create report file: %s", report_path.c_str());
            return;
        }
        report_ << "=== Self Stress Test Report ===" << std::endl;
        report_ << "Test Time: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
        report_ << "Port: " << port_ << std::endl;
        report_ << "Clients: " << clients_ << std::endl;
        report_ << "Duration: " << duration_ << "s" << std::endl;

        std::cout << "=== Self Stress Test ===" << std::endl;
        std::cout << "Port: " << port_ << std::endl;
        std::cout << "Clients: " << clients_ << std::endl;
        std::cout << "Duration: " << duration_ << "s" << std::endl;

        // 启动服务器线程
        std::thread server_thread([this] {
            runServer();
        });

        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // 运行客户端测试
        runClients();

        // 等待服务器线程结束
        LOG_INFO("Waiting for server thread to finish...");
        if (server_thread.joinable()) {
            server_thread.join();
        }
        LOG_INFO("Server thread finished");
    }

private:
    void runServer() {
        LOG_INFO("Server initialization started");
        
        server_loop_ = new EventLoop;
        InetAddress listenAddr(port_);
        server_ = new TcpServer(server_loop_, listenAddr);

        server_->setConnectionCallback([this](const TcpConnectionPtr& conn) {
            if (conn->connected()) {
                total_connections_.fetch_add(1);
                active_connections_.fetch_add(1);
                LOG_DEBUG("New connection established: %s, Total: %ld, Active: %ld", 
                          conn->name().c_str(), 
                          total_connections_.load(), 
                          active_connections_.load());
            } else {
                active_connections_.fetch_sub(1);
                LOG_DEBUG("Connection closed: %s, Total: %ld, Active: %ld", 
                          conn->name().c_str(), 
                          total_connections_.load(), 
                          active_connections_.load());
            }
        });

        server_->setMessageCallback([this](const TcpConnectionPtr& conn,
                                      Buffer* buf,
                                      Timestamp receiveTime) {
            conn->send(buf);
        });

        // 【参数调优】服务器线程数：4(默认) -> 8(高负荷)
        // 增加线程数可以提高并发处理能力
        server_->setThreadNum(8);
        LOG_INFO("Server thread count set to 8");
        server_->start();
        LOG_INFO("Server started on port %d", port_);

        server_loop_->runEvery(5.0, [this]() {
            LOG_INFO("=== Server Status ===");
            LOG_INFO("Total Connections: %ld", total_connections_.load());
            LOG_INFO("Active Connections: %ld", active_connections_.load());
        });

        LOG_INFO("Server loop started");
        server_loop_->loop();
        LOG_INFO("Server loop finished");

        LOG_INFO("Deleting server...");
        delete server_;
        server_ = nullptr;
        LOG_INFO("Deleting loop...");
        delete server_loop_;
        server_loop_ = nullptr;
        LOG_INFO("Server cleanup done");
    }

    void runClients() {
        LOG_INFO("Starting client connections");
        
        std::vector<std::thread> client_threads;
        std::atomic<int64_t> total_requests{0};
        std::atomic<int64_t> total_bytes{0};
        std::atomic<int64_t> failed_requests{0};
        std::atomic<bool> running{true};
        std::atomic<bool> all_connected{false};

        auto start_time = std::chrono::steady_clock::now();

        // 启动客户端线程
        for (int i = 0; i < clients_; ++i) {
            client_threads.emplace_back([this, &total_requests, &total_bytes, &failed_requests, &running, &all_connected] {
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) {
                    LOG_ERROR("Failed to create socket, errno: %d", errno);
                    return;
                }

                int reuse = 1;
                setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

                struct sockaddr_in serv_addr;
                memset(&serv_addr, 0, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(port_);
                inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

                if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                    LOG_ERROR("Failed to connect to server, errno: %d", errno);
                    close(sockfd);
                    return;
                }

                // 【参数调优】数据包大小：1024B(默认) -> 16384B(高负荷)
                // 增加数据包大小可以测试大负载下的性能表现
                const int payload_size = 16384;
                char* payload = new char[payload_size];
                memset(payload, 'A', payload_size);

                // 等待所有连接建立
                while (!all_connected.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                while (running.load()) {
                    // 发送数据
                    ssize_t sent = send(sockfd, payload, payload_size, 0);
                    if (sent < 0) {
                        failed_requests.fetch_add(1);
                        break;
                    }

                    // 接收数据
                    char recv_buf[payload_size];
                    size_t total_received = 0;
                    while (total_received < payload_size) {
                        ssize_t received = recv(sockfd, recv_buf + total_received,
                                             payload_size - total_received, 0);
                        if (received < 0) {
                            failed_requests.fetch_add(1);
                            break;
                        }
                        if (received == 0) {
                            // 连接关闭
                            failed_requests.fetch_add(1);
                            break;
                        }
                        total_received += received;
                    }

                    if (total_received == payload_size) {
                        total_requests.fetch_add(1);
                        total_bytes.fetch_add(payload_size * 2);
                    }
                }

                delete[] payload;
                close(sockfd);
            });
        }

        // 等待所有客户端连接
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        all_connected.store(true);
        LOG_INFO("All clients connected, starting test...");

        // 运行指定时间
        std::this_thread::sleep_for(std::chrono::seconds(duration_));
        running.store(false);

        // 等待所有客户端线程结束
        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        auto end_time = std::chrono::steady_clock::now();
        double duration = std::chrono::duration<double>(end_time - start_time).count();

        // 打印结果
        LOG_INFO("=== Test Results ===");
        LOG_INFO("Total Requests: %ld", total_requests.load());
        LOG_INFO("Failed Requests: %ld", failed_requests.load());
        LOG_INFO("Total Bytes: %ld bytes", total_bytes.load());
        LOG_INFO("Duration: %.2fs", duration);
        LOG_INFO("QPS: %.2f", total_requests.load() / duration);
        LOG_INFO("Throughput: %.2f MB/s", (total_bytes.load() / duration / 1024 / 1024));

        // 输出结果到文件
        report_ << "\n=== Test Results ===" << std::endl;
        report_ << "Total Requests: " << total_requests.load() << std::endl;
        report_ << "Failed Requests: " << failed_requests.load() << std::endl;
        report_ << "Total Bytes: " << total_bytes.load() << " bytes" << std::endl;
        report_ << "Duration: " << duration << "s" << std::endl;
        report_ << "QPS: " << total_requests.load() / duration << std::endl;
        report_ << "Throughput: " << (total_bytes.load() / duration / 1024 / 1024) << " MB/s" << std::endl;

        // 停止服务器
        std::cout << "Stopping server..." << std::endl;
        if (server_loop_) {
            server_loop_->quit();
        }
        std::cout << "Server stopped" << std::endl;

        // 关闭文件
        report_.close();
        std::cout << "Test results saved" << std::endl;
    }

    int port_;
    int clients_;
    int duration_;
    EventLoop* server_loop_;
    TcpServer* server_;
    std::atomic<int64_t> total_connections_;
    std::atomic<int64_t> active_connections_;
    std::ofstream report_;
};

int main(int argc, char* argv[]) {
    // 【参数调优】监听端口：8888(默认) -> 8888(高负荷)
    // 端口号保持不变，如需修改可在此处调整
    int port = 8888;
    
    // 【参数调优】客户端数量：100(默认) -> 5000(高负荷)
    // 增加客户端数量可以测试高并发下的性能表现
    int clients = 5000;
    
    // 【参数调优】测试持续时间：30s(默认) -> 60s(高负荷)
    // 延长测试时间可以获得更稳定的性能数据
    int duration = 60;

    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    if (argc > 2) {
        clients = std::atoi(argv[2]);
    }
    if (argc > 3) {
        duration = std::atoi(argv[3]);
    }

    LOG_INFO("SelfStressTest parameters: port=%d, clients=%d, duration=%d", port, clients, duration);

    SelfStressTest test(port, clients, duration);
    test.run();

    LOG_INFO("SelfStressTest main completed");

    // 检查是否有错误日志
    if (!hasErrorLog()) {
        std::cout << "No error logs found, cleaning up log files..." << std::endl;
        cleanupLogFiles();
        std::cout << "Log files cleaned up successfully." << std::endl;
    } else {
        std::cout << "Error logs found, keeping log files for inspection." << std::endl;
    }

    return 0;
}
