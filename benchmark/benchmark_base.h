#pragma once
#include "Timestamp.h"
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>

class BenchmarkBase {
public:
    struct Result {
        int64_t total_requests;
        int64_t total_bytes;
        double duration_seconds;
        double qps;
        double throughput_mbps;
        double avg_latency_ms;
        double p50_latency_ms;
        double p95_latency_ms;
        double p99_latency_ms;

        void print() const;
    };

    virtual void setup() = 0;
    virtual void teardown() = 0;
    virtual Result run(int concurrent_clients, int duration_seconds) = 0;

protected:
    void recordLatency(double latency_ms);
    Result calculateResult(int64_t total_requests, int64_t total_bytes,
                          double duration_seconds) const;

private:
    std::vector<double> latencies_;
    mutable std::mutex latencies_mutex_;
};


