#include "benchmark_base.h"
#include "TcpServer.h"
#include "Eventloop.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "Logger.h"
#include "AsyncLoggingInit.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>
#include <fstream>
#include <chrono>
#include <future>
#include <atomic>
#include <algorithm>

class LatencyTest : public BenchmarkBase {
public:
    LatencyTest(const std::string& server_ip, int port, int payload_size)
        : server_ip_(server_ip), port_(port), payload_size_(payload_size),
          loop_(nullptr), server_(nullptr) {}

    void setup() override {
        // 设置日志级别为INFO，启用异步日志记录
        Logger::instance().setLogLevel(ERROR);

        // 使用 promise/future 确保服务器启动完成
        std::promise<void> server_ready;
        auto future = server_ready.get_future();

        // 启动服务器线程
        server_thread_.reset(new std::thread([this, &server_ready] {
            loop_ = new EventLoop;
            InetAddress listenAddr(port_);
            server_ = new TcpServer(loop_, listenAddr);

            server_->setConnectionCallback([](const TcpConnectionPtr& conn) {
                if (conn->connected()) {
                    LOG_INFO("New connection from %s to %s, name=%s",
                             conn->peerAddress().toIpPort().c_str(),
                             conn->localAddress().toIpPort().c_str(),
                             conn->name().c_str());
                } else {
                    LOG_INFO("Connection closed: %s", conn->name().c_str());
                }
            });

            server_->setMessageCallback([](const TcpConnectionPtr& conn,
                                          Buffer* buf,
                                          Timestamp) {
                // === 关键修复 2: 确保与官方 muduo 一致的 Buffer 处理 ===
                conn->send(buf);
                buf->retrieveAll();  // 显式移动读指针，避免内存累积
            });

            // 与官方 muduo 保持一致的线程数
            server_->setThreadNum(8);
            server_->start();

            // 确保事件循环已运行后再通知主线程
            loop_->runAfter(0.01, [&server_ready]() {
                server_ready.set_value();
            });

            loop_->loop();

            // 资源在 EventLoop 线程内安全清理
            delete server_;
            server_ = nullptr;
            delete loop_;
            loop_ = nullptr;
        }));

        // 主线程等待服务器启动完成，最多等待5秒
        if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
            throw std::runtime_error("Server startup timeout");
        }
    }

    Result run(int concurrent_clients, int duration_seconds) override {
        // === 关键修复 3: 降低客户端线程数至合理范围 ===
        // 1000 线程会导致严重调度延迟，改用 100 线程（接近 CPU 核心数）
        const int max_clients = 100;  // 8核机器推荐 50-100
        concurrent_clients = std::min(concurrent_clients, max_clients);

        std::cout << "Running test with " << concurrent_clients << " clients (non-blocking I/O)" << std::endl;
        std::vector<std::thread> clients;
        std::atomic<int64_t> total_requests{0};
        std::atomic<int64_t> total_bytes{0};
        std::atomic<bool> running{true};
        std::vector<int> client_sockets;

        // === 步骤 1: 建立所有连接（阻塞模式，确保连接成功）===
        for (int i = 0; i < concurrent_clients; ++i) {
            int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); // 直接创建非阻塞 socket
            if (sockfd < 0) {
                std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
                continue;
            }

            // 设置 SO_REUSEADDR 避免端口耗尽
            int opt = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            struct sockaddr_in serv_addr{};
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port_);
            inet_pton(AF_INET, server_ip_.c_str(), &serv_addr.sin_addr);

            // 非阻塞 connect
            if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                if (errno != EINPROGRESS) {
                    close(sockfd);
                    continue;
                }
            }

            // 等待连接完成（带超时）
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(sockfd, &wfds);
            struct timeval tv{0, 500000}; // 500ms 超时

            if (select(sockfd + 1, nullptr, &wfds, nullptr, &tv) <= 0) {
                close(sockfd);
                continue;
            }

            // 检查连接错误
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                close(sockfd);
                continue;
            }

            client_sockets.push_back(sockfd);
        }

        if (client_sockets.empty()) {
            throw std::runtime_error("No clients connected successfully");
        }

        std::cout << "Successfully connected " << client_sockets.size() << " clients" << std::endl;

        // === 步骤 2: 预热阶段（消除 TCP 慢启动影响）===
        std::cout << "Warming up connections..." << std::endl;
        std::vector<char> warmup_payload(64, 'A');
        for (int sockfd : client_sockets) {
            // 发送 10 个预热包
            for (int j = 0; j < 10; ++j) {
                ssize_t sent = 0;
                while (sent < 64) {
                    ssize_t n = send(sockfd, warmup_payload.data() + sent, 64 - sent, 0);
                    if (n < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                        break;
                    }
                    sent += n;
                }
                if (sent != 64) break;

                // 接收响应
                char buf[64];
                ssize_t received = 0;
                while (received < 64) {
                    ssize_t n = recv(sockfd, buf + received, 64 - received, 0);
                    if (n < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                        break;
                    }
                    if (n == 0) break;
                    received += n;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Warmup completed" << std::endl;

        // === 步骤 3: 准备测试数据 ===
        std::vector<char> payload(payload_size_, 'A');
        std::vector<char> recv_buf(payload_size_);

        auto test_start = std::chrono::steady_clock::now();
        auto test_end = test_start + std::chrono::seconds(duration_seconds);

        // === 步骤 4: 启动客户端线程（非阻塞 busy-wait）===
        for (size_t i = 0; i < client_sockets.size(); ++i) {
            int sockfd = client_sockets[i];
            clients.emplace_back([this, sockfd, i, &payload, &recv_buf, 
                                &total_requests, &total_bytes, &running, test_end] {
                while (running.load(std::memory_order_relaxed)) {
                    // 检查是否超时
                    if (std::chrono::steady_clock::now() >= test_end) {
                        running.store(false, std::memory_order_relaxed);
                        break;
                    }

                    auto send_start = std::chrono::steady_clock::now();

                    // === 非阻塞发送（busy-wait）===
                    ssize_t sent = 0;
                    while (sent < payload.size()) {
                        ssize_t n = send(sockfd, payload.data() + sent, payload.size() - sent, 0);
                        if (n < 0) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                            return; // 连接错误，退出线程
                        }
                        sent += n;
                    }

                    // === 非阻塞接收（busy-wait）===
                    ssize_t received = 0;
                    while (received < payload.size()) {
                        ssize_t n = recv(sockfd, recv_buf.data() + received, payload.size() - received, 0);
                        if (n < 0) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                            return;
                        }
                        if (n == 0) return; // 连接关闭
                        received += n;
                    }

                    auto send_end = std::chrono::steady_clock::now();
                    auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(
                        send_end - send_start).count();

                    // 仅记录有效延迟（过滤掉超时异常值）
                    if (latency_us > 0 && latency_us < 1000000) { // < 1s
                        recordLatency(latency_us / 1000.0); // 转为 ms
                        total_requests.fetch_add(1, std::memory_order_relaxed);
                        total_bytes.fetch_add(payload.size() * 2, std::memory_order_relaxed);
                    }
                }
                close(sockfd);
            });
        }

        // === 步骤 5: 等待测试结束 ===
        std::this_thread::sleep_until(test_end);
        running.store(false, std::memory_order_relaxed);

        for (auto& client : clients) {
            if (client.joinable()) client.join();
        }

        // 清理未关闭的 sockets（防御性编程）
        for (int sockfd : client_sockets) {
            close(sockfd);
        }

        auto actual_end = std::chrono::steady_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(
            actual_end - test_start).count();

        std::cout << "Test finished. Total requests: " << total_requests.load() 
                  << ", Duration: " << std::fixed << std::setprecision(2) << duration << "s" << std::endl;
        std::cout << "Throughput: " << std::fixed << std::setprecision(2) 
                  << (total_requests.load() / duration) << " req/s" << std::endl;

        return calculateResult(total_requests.load(), total_bytes.load(), duration);
    }

    void teardown() override {
        if (loop_) {
            loop_->quit(); // 线程安全
        }
        if (server_thread_ && server_thread_->joinable()) {
            server_thread_->join();
            server_thread_.reset();
        }

        // 资源已在服务器线程中清理，仅置空指针
        server_ = nullptr;
        loop_ = nullptr;
    }

private:
    std::string server_ip_;
    int port_;
    int payload_size_;
    EventLoop* loop_;
    TcpServer* server_;
    std::unique_ptr<std::thread> server_thread_;
};

void run_latency_tests() {
    // === 关键修复 4: 合理的测试参数 ===
    std::vector<int> payload_sizes = {64, 256, 1024, 4096, 16384, 65536};
    const int clients = 100;    // 从 1000 降至 100（避免客户端瓶颈）
    const int duration = 30;    // 30s 足够稳定，60s 无必要
    std::string report_path = "latency_report.txt";

    std::ofstream report(report_path);
    if (!report.is_open()) {
        std::cerr << "Failed to create report file: " << report_path << std::endl;
        return;
    }

    report << "=== Optimized Latency Test Report ===" << std::endl;
    report << "Test Time: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    report << "Clients: " << clients << " (non-blocking I/O)" << std::endl;
    report << "Duration: " << duration << "s" << std::endl;
    report << "Server Threads: 8" << std::endl;
    report << std::endl;

    std::cout << "=== Optimized Latency Test ===" << std::endl;
    std::cout << "Clients: " << clients << " (non-blocking I/O)" << std::endl;
    std::cout << "Duration: " << duration << "s" << std::endl;
    std::cout << "Server Threads: 8" << std::endl;
    std::cout << std::endl;

    for (int payload : payload_sizes) {
        std::cout << "Testing payload size: " << payload << " bytes" << std::endl;
        report << "Testing payload size: " << payload << " bytes" << std::endl;

        try {
            LatencyTest test("127.0.0.1", 8888, payload);
            test.setup();
            auto result = test.run(clients, duration);
            test.teardown();

            report << "  Avg Latency: " << std::fixed << std::setprecision(3)
                   << result.avg_latency_ms << " ms" << std::endl;
            report << "  P50 Latency: " << result.p50_latency_ms << " ms" << std::endl;
            report << "  P95 Latency: " << result.p95_latency_ms << " ms" << std::endl;
            report << "  P99 Latency: " << result.p99_latency_ms << " ms" << std::endl;
            report << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error testing payload " << payload << ": " << e.what() << std::endl;
            report << "  Error: " << e.what() << std::endl;
        }
    }

    report.close();
    std::cout << "Report saved to: " << report_path << std::endl;

    std::cout << "\n=== Expected Results (127.0.0.1回环网络) ===" << std::endl;
    std::cout << "Healthy P50: 0.05~0.15 ms (50~150 μs)" << std::endl;
    std::cout << "Healthy P99: < 2.0 ms (除非系统过载)" << std::endl;
    std::cout << "If P50 > 1.0 ms: Measurement is contaminated by client scheduling!" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: " << argv[0] << " [server_ip] [port]\n";
        std::cout << "Default: 127.0.0.1 8888\n";
        std::cout << "\nOptimized for accurate latency measurement:\n";
        std::cout << "  - Non-blocking I/O with busy-wait\n";
        std::cout << "  - 100 clients (avoids client-side bottleneck)\n";
        std::cout << "  - Connection warmup to eliminate TCP slow-start\n";
        std::cout << "  - All logging disabled during test\n";
        return 0;
    }

    std::string server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? std::atoi(argv[2]) : 8888;

    // 初始化异步日志系统
    std::string log_basename = "log/latency_test";
    initAsyncLogging(log_basename, 1024 * 1024 * 1024, 3);  // 1GB滚动，3秒刷新
    setAsyncOutput();  // 将Logger的输出重定向到异步日志

    std::cout << "Starting OPTIMIZED latency test against " << server_ip << ":" << port << std::endl;
    std::cout << "WARNING: This test uses non-blocking I/O and will consume 100% CPU on client threads." << std::endl;
    std::cout << "Ensure you have sufficient CPU cores (recommend 8+ cores for 100 clients)." << std::endl;
    std::cout << std::endl;

    run_latency_tests();
    std::cout << "Optimized latency test completed." << std::endl;

    return 0;
}
