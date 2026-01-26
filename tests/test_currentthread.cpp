
#include "CurrentThread.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>

void test_current_thread_tid() {
    // 获取当前线程的tid
    int tid = CurrentThread::tid();
    std::cout << "Current thread ID: " << tid << std::endl;
    assert(tid > 0);
    std::cout << "CurrentThread::tid() test passed" << std::endl;
}

void test_multiple_threads() {
    const int numThreads = 5;
    std::vector<std::thread> threads;
    std::vector<int> tids(numThreads);

    // 创建多个线程，每个线程获取自己的tid
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &tids]() {
            tids[i] = CurrentThread::tid();
            std::cout << "Thread " << i << " ID: " << tids[i] << std::endl;
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 验证所有线程的tid都不同
    for (int i = 0; i < numThreads; ++i) {
        for (int j = i + 1; j < numThreads; ++j) {
            assert(tids[i] != tids[j]);
        }
    }

    std::cout << "Multiple threads test passed" << std::endl;
}

int main() {
    std::cout << "=== CurrentThread Tests ===" << std::endl;
    test_current_thread_tid();
    test_multiple_threads();
    std::cout << "=== All CurrentThread Tests Passed ===" << std::endl;
    return 0;
}
