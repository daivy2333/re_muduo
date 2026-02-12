#include "benchmark_base.h"
#include "echo_server_bench.h"
#include "AsyncLoggingInit.h"
#include <fstream>
#include <iomanip>
#include <iostream>

void run_throughput_benchmark() {
    // 【参数调优】客户端数量：{1, 10, 100, 1000}(默认) -> {10, 100, 500, 1000, 2000}(高负荷)
    // 增加客户端数量可以测试高并发下的性能表现
    std::vector<int> client_counts = {10, 100, 500, 1000, 2000};
    
    // 【参数调优】数据包大小：{64, 1024, 4096, 16384}(默认) -> {64, 1024, 4096, 16384, 32768, 65536}(高负荷)
    // 增加数据包大小可以测试大负载下的性能表现
    std::vector<int> payload_sizes = {64, 1024, 4096, 16384, 32768, 65536};

    std::string report_path = "/home/daivy/projects/muduo_learn/re_muduo/throughput_report.txt";
    std::ofstream report(report_path);
    if (!report.is_open()) {
        std::cerr << "Failed to create report file: " << report_path << std::endl;
        return;
    }
    report << "=== Throughput Benchmark Report ===" << std::endl;
    report << "Test Time: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    report << std::endl;

    for (int clients : client_counts) {
        for (int payload : payload_sizes) {
            EchoServerBench bench(8888, payload);
            bench.setup();

            // 【参数调优】测试持续时间：10s(默认) -> 30s(高负荷)
            // 延长测试时间可以获得更稳定的性能数据
            auto result = bench.run(clients, 30);  // 运行30秒

            report << "Clients: " << clients << std::endl;
            report << "Payload: " << payload << " bytes" << std::endl;
            report << "  QPS: " << std::fixed << std::setprecision(2) << result.qps << std::endl;
            report << "  Throughput: " << result.throughput_mbps << " MB/s" << std::endl;
            report << "  Avg Latency: " << result.avg_latency_ms << " ms" << std::endl;
            report << "  P95 Latency: " << result.p95_latency_ms << " ms" << std::endl;
            report << "  P99 Latency: " << result.p99_latency_ms << " ms" << std::endl;
            report << std::endl;

            bench.teardown();

            std::cout << "Clients: " << clients
                      << ", Payload: " << payload
                      << "B, QPS: " << result.qps
                      << ", Throughput: " << result.throughput_mbps << " MB/s\n";
        }
    }

    report.close();
    std::cout << "Report saved to: " << report_path << std::endl;
}

int main() {
    std::cout << "Starting throughput benchmark...\n";
    run_throughput_benchmark();
    std::cout << "Throughput benchmark completed." << std::endl;

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
