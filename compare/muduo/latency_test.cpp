#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/Buffer.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"
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
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>

using namespace muduo;
using namespace muduo::net;

// 从 benchmark_base.h 复制必要的定义
class BenchmarkBase {
public:
    struct Result {
        double avg_latency_ms;
        double p50_latency_ms;
        double p95_latency_ms;
        double p99_latency_ms;
    };

    virtual ~BenchmarkBase() = default;
    virtual void setup() = 0;
    virtual Result run(int concurrent_clients, int duration_seconds) = 0;
    virtual void teardown() = 0;

protected:
    std::vector<double> latencies_;

    void recordLatency(double latency_ms) {
        std::lock_guard<std::mutex> lock(latencies_mutex_);
        latencies_.push_back(latency_ms);
    }

    Result calculateResult(int64_t total_requests, int64_t total_bytes, double duration) {
        Result result;

        if (!latencies_.empty()) {
            std::lock_guard<std::mutex> lock(latencies_mutex_);
            std::sort(latencies_.begin(), latencies_.end());

            double sum = 0;
            for (auto latency : latencies_) {
                sum += latency;
            }
            result.avg_latency_ms = sum / latencies_.size();

            size_t p50_idx = latencies_.size() * 0.5;
            size_t p95_idx = latencies_.size() * 0.95;
            size_t p99_idx = latencies_.size() * 0.99;

            result.p50_latency_ms = latencies_[p50_idx];
            result.p95_latency_ms = latencies_[p95_idx];
            result.p99_latency_ms = latencies_[p99_idx];
        }

        return result;
    }

    static double timeDifference(Timestamp high, Timestamp low) {
        return muduo::timeDifference(high, low);
    }

private:
    std::mutex latencies_mutex_;
};

class MuduoLatencyTest : public BenchmarkBase {
public:
    MuduoLatencyTest(const std::string& server_ip, int port, int payload_size)
        : server_ip_(server_ip), port_(port), payload_size_(payload_size),
          loop_(nullptr), server_(nullptr) {}

    ~MuduoLatencyTest() {
        teardown();
    }

    void setup() override {
        Logger::setLogLevel(Logger::WARN);  // 降低日志级别提高性能

        std::promise<bool> server_ready_promise;
        std::future<bool> server_ready = server_ready_promise.get_future();

        server_thread_.reset(new std::thread([this, &server_ready_promise] {
            try {
                loop_ = new EventLoop;

                InetAddress listenAddr(port_);
                server_ = new TcpServer(loop_, listenAddr, "MuduoLatencyTest");

                server_->setConnectionCallback([this](const TcpConnectionPtr& conn) {
                    // 简化连接回调，不访问conn对象
                    if (conn->connected()) {
                        // LOG_DEBUG << "New connection";
                    }
                });

                server_->setMessageCallback([this](const TcpConnectionPtr& conn,
                                                  Buffer* buf,
                                                  muduo::Timestamp receiveTime) {
                    // 使用最简单的echo逻辑
                    conn->send(buf->peek(), buf->readableBytes());
                    buf->retrieveAll();
                });

                // 将线程数设置为8，与benchmark保持一致
                server_->setThreadNum(8);

                server_->start();

                // 确保服务器已经真正启动
                loop_->runAfter(0.1, [&server_ready_promise]() {
                    server_ready_promise.set_value(true);
                });

                // EventLoop 主循环
                loop_->loop();  // ← 退出点：quit() 被调用后从此处返回

                // ===== 关键修复：在 EventLoop 线程内清理资源 =====
                // 此时所有事件已处理完毕，安全析构
                LOG_INFO << "Cleaning up resources in EventLoop thread";
                delete server_;  // ✓ 在正确线程析构
                server_ = nullptr;
                delete loop_;    // ✓ 在正确线程析构
                loop_ = nullptr;
                // ==============================================

            } catch (const std::exception& e) {
                server_ready_promise.set_exception(std::make_exception_ptr(e));
                // 异常时也需清理资源
                delete server_;
                delete loop_;
                server_ = nullptr;
                loop_ = nullptr;
            } catch (...) {
                server_ready_promise.set_exception(
                    std::make_exception_ptr(std::runtime_error("Unknown error")));
                // 异常时也需清理资源
                delete server_;
                delete loop_;
                server_ = nullptr;
                loop_ = nullptr;
            }
        }));

        // 等待服务器启动
        try {
            if (server_ready.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
                throw std::runtime_error("Server startup timeout");
            }
            server_ready.get();
        } catch (const std::exception& e) {
            teardown();
            throw;
        }
    }

    Result run(int concurrent_clients, int duration_seconds) override {
        // 限制最大并发数，与benchmark保持一致
        const int max_clients = 1000;  // 与benchmark保持一致
        concurrent_clients = std::min(concurrent_clients, max_clients);

        std::cout << "Running test with " << concurrent_clients << " clients" << std::endl;

        std::vector<std::thread> clients;
        std::atomic<int64_t> total_requests{0};
        std::atomic<int64_t> total_bytes{0};
        std::atomic<bool> running{true};
        std::atomic<int> connected_clients{0};

        std::vector<int> client_sockets;
        client_sockets.reserve(concurrent_clients);

        // 先建立所有连接
        for (int i = 0; i < concurrent_clients; ++i) {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                std::cerr << "Failed to create socket for client " << i
                         << ": " << strerror(errno) << std::endl;
                continue;
            }

            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port_);
            inet_pton(AF_INET, server_ip_.c_str(), &serv_addr.sin_addr);

            // 设置非阻塞连接
            int flags = fcntl(sockfd, F_GETFL, 0);
            fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

            int ret = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            if (ret < 0 && errno != EINPROGRESS) {
                std::cerr << "Connect failed for client " << i
                         << ": " << strerror(errno) << std::endl;
                close(sockfd);
                continue;
            }

            client_sockets.push_back(sockfd);
        }

        // 等待所有连接完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 验证连接
        for (auto it = client_sockets.begin(); it != client_sockets.end(); ) {
            int sockfd = *it;
            int error = 0;
            socklen_t len = sizeof(error);

            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                std::cerr << "Connection failed for socket " << sockfd << std::endl;
                close(sockfd);
                it = client_sockets.erase(it);
            } else {
                connected_clients++;
                it++;
            }
        }

        std::cout << "Successfully connected " << connected_clients << " clients" << std::endl;

        // 准备payload
        std::vector<char> payload(payload_size_, 'A');
        std::vector<char> recv_buf(payload_size_);

        auto start_time = std::chrono::steady_clock::now();

        // 为每个连接启动测试线程
        for (size_t i = 0; i < client_sockets.size(); ++i) {
            int sockfd = client_sockets[i];
            clients.emplace_back([this, sockfd, i, &payload, &recv_buf,
                                  &total_requests, &total_bytes, &running] {
                int request_count = 0;
                const char* send_data = payload.data();

                while (running.load()) {
                    try {
                        auto send_start = std::chrono::steady_clock::now();

                        // 发送
                        ssize_t sent = 0;
                        while (sent < payload_size_) {
                            ssize_t n = send(sockfd, send_data + sent, payload_size_ - sent, 0);
                            if (n < 0) {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                    continue;
                                }
                                throw std::runtime_error("Send failed: " + std::string(strerror(errno)));
                            }
                            sent += n;
                        }

                        // 接收
                        ssize_t received = 0;
                        while (received < payload_size_) {
                            ssize_t n = recv(sockfd, recv_buf.data() + received,
                                           payload_size_ - received, 0);
                            if (n < 0) {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                    continue;
                                }
                                throw std::runtime_error("Recv failed: " + std::string(strerror(errno)));
                            }
                            if (n == 0) {
                                throw std::runtime_error("Connection closed");
                            }
                            received += n;
                        }

                        auto send_end = std::chrono::steady_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                            send_end - send_start);
                        double latency_ms = duration.count() / 1000.0;

                        recordLatency(latency_ms);
                        total_requests.fetch_add(1);
                        total_bytes.fetch_add(payload_size_ * 2);
                        request_count++;

                    } catch (const std::exception& e) {
                        std::cerr << "Client " << i << " error: " << e.what() << std::endl;
                        break;
                    } catch (...) {
                        std::cerr << "Client " << i << " unknown error" << std::endl;
                        break;
                    }
                }

                close(sockfd);
            });
        }

        // 运行测试
        std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));
        running.store(false);

        // 等待所有客户端线程结束
        for (auto& client : clients) {
            if (client.joinable()) client.join();
        }

        auto end_time = std::chrono::steady_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(
            end_time - start_time).count();

        std::cout << "Test finished. Total requests: " << total_requests.load()
                 << ", Duration: " << duration << "s" << std::endl;

        return calculateResult(total_requests.load(), total_bytes.load(), duration);
    }

    // 新增：保存延迟数据到文件
    void saveLatencyData(const std::string& filename) {
        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        std::lock_guard<std::mutex> lock(latencies_mutex_);
        for (double latency : latencies_) {
            out << std::fixed << std::setprecision(6) << latency << "\n";
        }
        out.close();
    }

    void teardown() override {
        // 1. 安全地请求 EventLoop 退出（quit() 是线程安全的）
        if (loop_) {
            loop_->quit();  // ✓ 线程安全，从任何线程调用都安全
        }

        // 2. 等待服务器线程结束
        if (server_thread_ && server_thread_->joinable()) {
            server_thread_->join();
        }

        // 3. 注意：资源已在服务器线程中删除，此处仅置空指针
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
    std::mutex latencies_mutex_;  // 用于保护 latencies_ 的访问
};

void run_muduo_latency_tests() {
    // 测试不同数据包大小，与benchmark保持一致
    std::vector<int> payload_sizes = {64, 256, 1024, 4096, 16384, 65536};
    const int clients = 1000;  // 与benchmark保持一致
    const int duration = 60;  // 与benchmark保持一致

    std::string report_path = "muduo_latency_report.txt";
    std::ofstream report(report_path);
    if (!report.is_open()) {
        std::cerr << "Failed to create report file: " << report_path << std::endl;
        return;
    }

    report << "=== Muduo Latency Test Report ===" << std::endl;
    report << "Test Time: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    report << "Clients: " << clients << std::endl;
    report << "Duration: " << duration << "s" << std::endl;
    report << std::endl;

    std::cout << "=== Muduo Latency Test ===" << std::endl;
    std::cout << "Clients: " << clients << std::endl;
    std::cout << "Duration: " << duration << "s" << std::endl;
    std::cout << std::endl;

    for (int payload : payload_sizes) {
        std::cout << "Testing payload size: " << payload << " bytes" << std::endl;
        report << "Testing payload size: " << payload << " bytes" << std::endl;

        try {
            MuduoLatencyTest test("127.0.0.1", 8888, payload);
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

            // 保存原始延迟数据到文件
            //std::string data_file = "muduo_latency_data_" + std::to_string(payload) + ".dat";
            //test.saveLatencyData(data_file);
            //std::cout << "  Latency data saved to: " << data_file << std::endl;
            //report << "  Latency data saved to: " << data_file << std::endl;
            // report << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error testing payload " << payload << ": " << e.what() << std::endl;
            report << "  Error: " << e.what() << std::endl;
        }
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

    std::cout << "Starting muduo latency test against " << server_ip << ":" << port << std::endl;
    run_muduo_latency_tests();
    std::cout << "Muduo latency test completed." << std::endl;

    return 0;
}
