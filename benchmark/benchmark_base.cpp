#include "benchmark_base.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

void BenchmarkBase::Result::print() const {
    std::cout << "=== Benchmark Result ===" << std::endl;
    std::cout << "Total Requests: " << total_requests << std::endl;
    std::cout << "Total Bytes: " << total_bytes << std::endl;
    std::cout << "Duration: " << duration_seconds << "s" << std::endl;
    std::cout << "QPS: " << std::fixed << std::setprecision(2) << qps << std::endl;
    std::cout << "Throughput: " << throughput_mbps << " MB/s" << std::endl;
    std::cout << "Avg Latency: " << avg_latency_ms << " ms" << std::endl;
    std::cout << "P50 Latency: " << p50_latency_ms << " ms" << std::endl;
    std::cout << "P95 Latency: " << p95_latency_ms << " ms" << std::endl;
    std::cout << "P99 Latency: " << p99_latency_ms << " ms" << std::endl;
}

void BenchmarkBase::recordLatency(double latency_ms) {
    std::lock_guard<std::mutex> lock(latencies_mutex_);
    latencies_.push_back(latency_ms);
}

BenchmarkBase::Result BenchmarkBase::calculateResult(
    int64_t total_requests, int64_t total_bytes, double duration_seconds) const {
    Result result;
    result.total_requests = total_requests;
    result.total_bytes = total_bytes;
    result.duration_seconds = duration_seconds;

    if (duration_seconds > 0) {
        result.qps = static_cast<double>(total_requests) / duration_seconds;
        result.throughput_mbps = (static_cast<double>(total_bytes) / (1024.0 * 1024.0)) / duration_seconds;
    } else {
        result.qps = 0.0;
        result.throughput_mbps = 0.0;
    }

    // 计算延迟统计
    std::lock_guard<std::mutex> lock(latencies_mutex_);
    if (!latencies_.empty()) {
        std::vector<double> sorted_latencies = latencies_;
        std::sort(sorted_latencies.begin(), sorted_latencies.end());

        double sum = 0.0;
        for (double latency : sorted_latencies) {
            sum += latency;
        }
        result.avg_latency_ms = sum / sorted_latencies.size();

        size_t n = sorted_latencies.size();
        result.p50_latency_ms = sorted_latencies[n * 50 / 100];
        result.p95_latency_ms = sorted_latencies[n * 95 / 100];
        result.p99_latency_ms = sorted_latencies[n * 99 / 100];
    } else {
        result.avg_latency_ms = 0.0;
        result.p50_latency_ms = 0.0;
        result.p95_latency_ms = 0.0;
        result.p99_latency_ms = 0.0;
    }

    return result;
}


