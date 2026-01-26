#pragma once
#include "TcpServer.h"
#include "Eventloop.h"
#include "benchmark_base.h"
#include <atomic>
#include <thread>
#include <memory>

class EchoServerBench : public BenchmarkBase {
public:
    EchoServerBench(int port, int payload_size);

    void setup() override;
    BenchmarkBase::Result run(int concurrent_clients, int duration_seconds) override;
    void teardown() override;

private:
    int port_;
    int payload_size_;
    EventLoop* loop_;
    TcpServer* server_;
    std::unique_ptr<std::thread> server_thread_;
    std::atomic<int64_t> received_bytes_;
};
