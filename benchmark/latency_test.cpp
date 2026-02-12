#include "benchmark_base.h"
#include "TcpServer.h"
#include "Eventloop.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "AsyncLoggingInit.h"
#include "Logger.h"
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

class LatencyTest : public BenchmarkBase {
public:
    LatencyTest(const std::string& server_ip, int port, int payload_size)
        : server_ip_(server_ip), port_(port), payload_size_(payload_size), first_print_(true),
          loop_(nullptr), server_(nullptr) {}

    void setup() override {
        // 初始化异步日志
        initAsyncLogging("log/latency_test", 1024 * 1024 * 100, 3);  // 100MB滚动, 3秒刷新
        setAsyncOutput();
        
        LOG_INFO("LatencyTest setup started, server_ip=%s, port=%d, payload_size=%d", 
                 server_ip_.c_str(), port_, payload_size_);
        
        // 启动服务器线程
        server_thread_.reset(new std::thread([this] {
            loop_ = new EventLoop;
            InetAddress listenAddr(port_);
            server_ = new TcpServer(loop_, listenAddr);

            server_->setConnectionCallback([this](const TcpConnectionPtr& conn) {
                if (conn->connected()) {
                    LOG_DEBUG("New connection: %s", conn->name().c_str());
                }
            });

            server_->setMessageCallback([this](const TcpConnectionPtr& conn,
                                              Buffer* buf,
                                              Timestamp receiveTime) {
                // Echo back the received data
                conn->send(buf);
            });

            // 【参数调优】服务器线程数：4(默认) -> 8(高负荷)
            // 增加线程数可以提高并发处理能力
            server_->setThreadNum(8);
            LOG_INFO("Server thread count set to 8");
            server_->start();
            LOG_INFO("Server started on port %d", port_);

            loop_->loop();

            // 清理
            delete server_;
            server_ = nullptr;
            delete loop_;
            loop_ = nullptr;
        }));

        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Result run(int concurrent_clients, int duration_seconds) override {
        std::vector<std::thread> clients;
        std::atomic<int64_t> total_requests{0};
        std::atomic<int64_t> total_bytes{0};
        std::atomic<bool> running{true};

        auto start_time = Timestamp::now();

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
                inet_pton(AF_INET, server_ip_.c_str(), &serv_addr.sin_addr);

                if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                    LOG_ERROR("Connection failed: %s", strerror(errno));
                    close(sockfd);
                    return;
                }
                LOG_DEBUG("Client connected successfully");

                // 使用阻塞socket，简化延迟测试逻辑
                // int flags = fcntl(sockfd, F_GETFL, 0);
                // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

                char* payload = new char[payload_size_];
                memset(payload, 'A', payload_size_);

                while (running.load()) {
                    auto send_start = Timestamp::now();

                    ssize_t sent = send(sockfd, payload, payload_size_, 0);
                    if (sent < 0) {
                        std::cerr << "Send failed: " << strerror(errno) << std::endl;
                        break;
                    }
                    if (sent != payload_size_) {
                        std::cerr << "Partial send: " << sent << " / " << payload_size_ << std::endl;
                    }

                    char recv_buf[payload_size_];
                    size_t total_received = 0;
                    while (total_received < payload_size_) {
                        ssize_t received = recv(sockfd, recv_buf + total_received, 
                                             payload_size_ - total_received, 0);
                        if (received < 0) {
                            std::cerr << "Recv failed: " << strerror(errno) << std::endl;
                            break;
                        }
                        if (received == 0) {
                            std::cerr << "Connection closed by server" << std::endl;
                            break;
                        }
                        total_received += received;
                    }
                    if (total_received != payload_size_) {
                        std::cerr << "Incomplete receive: " << total_received << " / " << payload_size_ << std::endl;
                    }

                    auto send_end = Timestamp::now();
                    double latency = timeDifference(send_end, send_start) * 1000;
                    recordLatency(latency);
                    
                    // 只在第一次请求时打印延迟
                    if (first_print_.exchange(false)) {
                        std::cout << "First request latency: " << latency << " ms" << std::endl;
                    }

                    total_requests.fetch_add(1);
                    total_bytes.fetch_add(payload_size_ * 2);
                }

                delete[] payload;
                close(sockfd);
            });
        }

        std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));
        running.store(false);

        for (auto& client : clients) {
            if (client.joinable()) client.join();
        }

        auto end_time = Timestamp::now();
        double duration = timeDifference(end_time, start_time);

        return calculateResult(total_requests.load(), total_bytes.load(), duration);
    }

    void teardown() override {
        LOG_INFO("LatencyTest teardown started");
        
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
        LOG_INFO("LatencyTest teardown completed");
    }

private:
    std::string server_ip_;
    int port_;
    int payload_size_;
    std::atomic<bool> first_print_;
    EventLoop* loop_;
    TcpServer* server_;
    std::unique_ptr<std::thread> server_thread_;
};

void run_latency_tests() {
    // 【参数调优】数据包大小：{64, 256, 1024, 4096, 16384}(默认) -> {64, 256, 1024, 4096, 16384, 65536}(高负荷)
    // 增加更大的数据包可以测试大负载下的延迟表现
    std::vector<int> payload_sizes = {64, 256, 1024, 4096, 16384, 65536};
    
    // 【参数调优】并发客户端数：100(默认) -> 1000(高负荷)
    // 增加并发客户端数可以测试高并发下的延迟表现
    const int clients = 1000;
    
    // 【参数调优】测试持续时间：30s(默认) -> 60s(高负荷)
    // 延长测试时间可以获得更稳定的延迟统计数据
    const int duration = 60;

    // 创建输出文件
    std::string report_path = "/home/daivy/projects/muduo_learn/re_muduo/latency_report.txt";
    std::ofstream report(report_path);
    if (!report.is_open()) {
        std::cerr << "Failed to create report file: " << report_path << std::endl;
        return;
    }
    report << "=== Latency Test Report ===" << std::endl;
    report << "Test Time: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    report << "Clients: " << clients << std::endl;
    report << "Duration: " << duration << "s" << std::endl;
    report << std::endl;

    std::cout << "=== Latency Test ===" << std::endl;
    std::cout << "Clients: " << clients << std::endl;
    std::cout << "Duration: " << duration << "s" << std::endl;
    std::cout << std::endl;

    for (int payload : payload_sizes) {
        std::cout << "Testing payload size: " << payload << " bytes" << std::endl;
        report << "Testing payload size: " << payload << " bytes" << std::endl;

        LatencyTest test("127.0.0.1", 8888, payload);
        test.setup();
        auto result = test.run(clients, duration);
        test.teardown();

        std::cout << "  Avg Latency: " << std::fixed << std::setprecision(3) 
                  << result.avg_latency_ms << " ms" << std::endl;
        std::cout << "  P50 Latency: " << result.p50_latency_ms << " ms" << std::endl;
        std::cout << "  P95 Latency: " << result.p95_latency_ms << " ms" << std::endl;
        std::cout << "  P99 Latency: " << result.p99_latency_ms << " ms" << std::endl;
        std::cout << std::endl;

        report << "  Avg Latency: " << std::fixed << std::setprecision(3)
               << result.avg_latency_ms << " ms" << std::endl;
        report << "  P50 Latency: " << result.p50_latency_ms << " ms" << std::endl;
        report << "  P95 Latency: " << result.p95_latency_ms << " ms" << std::endl;
        report << "  P99 Latency: " << result.p99_latency_ms << " ms" << std::endl;
        report << std::endl;
    }
    report.close();
    std::cout << "Report saved to: " << report_path << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: " << argv[0] << " [server_ip] [port]\n";
        std::cout << "Default: 127.0.0.1 8888\n";
        return 0;
    }

    std::string server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? std::atoi(argv[2]) : 8888;

    std::cout << "Starting latency test against " << server_ip << ":" << port << std::endl;
    run_latency_tests();
    std::cout << "Latency test completed." << std::endl;

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
