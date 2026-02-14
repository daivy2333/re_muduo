
#include "CurrentThread.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <set>

// 测试基本功能：获取当前线程ID
void test_current_thread_tid() {
    std::cout << "Test 1: Basic tid() functionality" << std::endl;
    
    int tid = CurrentThread::tid();
    std::cout << "Current thread ID: " << tid << std::endl;
    assert(tid > 0);
    
    // 多次调用应该返回相同的值
    int tid2 = CurrentThread::tid();
    assert(tid == tid2);
    
    std::cout << "CurrentThread::tid() test passed" << std::endl;
}

// 测试多个线程的ID唯一性
void test_multiple_threads() {
    std::cout << "Test 2: Multiple threads have unique TIDs" << std::endl;
    
    const int numThreads = 10;
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
    std::set<int> uniqueTids(tids.begin(), tids.end());
    assert(uniqueTids.size() == static_cast<size_t>(numThreads));

    std::cout << "Multiple threads test passed" << std::endl;
}

// 测试线程局部存储的隔离性
void test_thread_local_storage() {
    std::cout << "Test 3: Thread local storage isolation" << std::endl;
    
    std::atomic<int> counter{0};
    const int numIterations = 100;
    
    std::thread t1([&counter]() {
        int myTid = CurrentThread::tid();
        for (int i = 0; i < numIterations; ++i) {
            assert(CurrentThread::tid() == myTid);
            counter++;
        }
    });
    
    std::thread t2([&counter]() {
        int myTid = CurrentThread::tid();
        for (int i = 0; i < numIterations; ++i) {
            assert(CurrentThread::tid() == myTid);
            counter++;
        }
    });
    
    t1.join();
    t2.join();
    
    assert(counter == numIterations * 2);
    std::cout << "Thread local storage isolation test passed" << std::endl;
}

// 测试嵌套线程
void test_nested_threads() {
    std::cout << "Test 4: Nested threads" << std::endl;
    
    int parentTid = CurrentThread::tid();
    int childTid = -1;
    int grandChildTid = -1;
    
    std::thread parent([&parentTid, &childTid, &grandChildTid]() {
        childTid = CurrentThread::tid();
        assert(childTid != parentTid);
        
        std::thread child([&childTid, &grandChildTid, parentTid]() {
            grandChildTid = CurrentThread::tid();
            assert(grandChildTid != childTid);
            assert(grandChildTid != parentTid);
        });
        
        child.join();
    });
    
    parent.join();
    
    assert(childTid > 0);
    assert(grandChildTid > 0);
    assert(parentTid != childTid);
    assert(parentTid != grandChildTid);
    assert(childTid != grandChildTid);
    
    std::cout << "Nested threads test passed" << std::endl;
}

// 测试线程ID的稳定性
void test_tid_stability() {
    std::cout << "Test 5: TID stability across multiple calls" << std::endl;
    
    const int numCalls = 1000;
    int firstTid = CurrentThread::tid();
    
    for (int i = 0; i < numCalls; ++i) {
        assert(CurrentThread::tid() == firstTid);
    }
    
    std::cout << "TID stability test passed" << std::endl;
}

// 测试大量并发线程
void test_massive_concurrent_threads() {
    std::cout << "Test 6: Massive concurrent threads" << std::endl;
    
    const int numThreads = 100;
    std::vector<std::thread> threads;
    std::vector<int> tids(numThreads);
    std::atomic<int> readyCount{0};
    
    // 创建大量线程
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &tids, &readyCount]() {
            tids[i] = CurrentThread::tid();
            readyCount++;
            // 等待所有线程都准备好
            while (readyCount < numThreads) {
                std::this_thread::yield();
            }
            // 再次验证tid
            assert(CurrentThread::tid() == tids[i]);
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 验证所有tid都不同
    std::set<int> uniqueTids(tids.begin(), tids.end());
    assert(uniqueTids.size() == static_cast<size_t>(numThreads));
    
    std::cout << "Massive concurrent threads test passed" << std::endl;
}

// 测试线程复用场景
void test_thread_reuse() {
    std::cout << "Test 7: Thread reuse scenario" << std::endl;
    
    std::vector<int> tids;
    const int numIterations = 5;
    
    for (int iter = 0; iter < numIterations; ++iter) {
        std::thread t([&tids]() {
            tids.push_back(CurrentThread::tid());
        });
        t.join();
    }
    
    // 验证每个线程的tid
    for (size_t i = 0; i < tids.size(); ++i) {
        assert(tids[i] > 0);
        std::cout << "Iteration " << i << " TID: " << tids[i] << std::endl;
    }
    
    std::cout << "Thread reuse test passed" << std::endl;
}

// 测试主线程ID
void test_main_thread() {
    std::cout << "Test 8: Main thread TID" << std::endl;
    
    int mainTid = CurrentThread::tid();
    assert(mainTid > 0);
    std::cout << "Main thread ID: " << mainTid << std::endl;
    
    // 在子线程中验证主线程ID不同
    int childTid = -1;
    std::thread t([&childTid, mainTid]() {
        childTid = CurrentThread::tid();
        assert(childTid != mainTid);
    });
    t.join();
    
    assert(childTid > 0);
    assert(childTid != mainTid);
    
    std::cout << "Main thread test passed" << std::endl;
}

// 测试线程ID的性能
void test_tid_performance() {
    std::cout << "Test 9: TID retrieval performance" << std::endl;
    
    const int numCalls = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numCalls; ++i) {
        volatile int tid = CurrentThread::tid();
        (void)tid; // 防止被优化掉
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Retrieved TID " << numCalls << " times in " 
              << duration.count() << " microseconds" << std::endl;
    std::cout << "Average: " << (duration.count() / static_cast<double>(numCalls)) 
              << " microseconds per call" << std::endl;
    
    std::cout << "TID performance test passed" << std::endl;
}

// 测试线程ID的边界情况
void test_tid_edge_cases() {
    std::cout << "Test 10: TID edge cases" << std::endl;
    
    // 测试tid()在异常情况下的行为
    try {
        int tid = CurrentThread::tid();
        assert(tid > 0);
        
        // 在try-catch块中再次调用
        int tid2 = CurrentThread::tid();
        assert(tid == tid2);
        
        throw std::runtime_error("Test exception");
    } catch (const std::exception& e) {
        // 在catch块中调用tid()
        int tid3 = CurrentThread::tid();
        assert(tid3 > 0);
    }
    
    std::cout << "TID edge cases test passed" << std::endl;
}

int main() {
    std::cout << "=== CurrentThread Tests ===" << std::endl;
    
    test_current_thread_tid();
    test_multiple_threads();
    test_thread_local_storage();
    test_nested_threads();
    test_tid_stability();
    test_massive_concurrent_threads();
    test_thread_reuse();
    test_main_thread();
    test_tid_performance();
    test_tid_edge_cases();
    
    std::cout << "=== All CurrentThread Tests Passed ===" << std::endl;
    return 0;
}
