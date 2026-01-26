#include "Thread.h"
#include "CurrentThread.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>
#include <atomic>


// 测试线程的基本创建和启动
void test_thread_creation() {
    std::cout << "=== Test Thread Creation ===" << std::endl;

    bool flag = false;
    Thread thread([&flag]() {
        flag = true;
    }, "TestThread");

    assert(!thread.started());
    assert(thread.name() == "TestThread");

    thread.start();
    assert(thread.started());

    thread.join();
    assert(flag);

    std::cout << "Thread creation test passed" << std::endl;
}

// 测试线程ID获取
void test_thread_id() {
    std::cout << "=== Test Thread ID ===" << std::endl;

    Thread thread([]() {
        std::cout << "Thread running with TID: " << CurrentThread::tid() << std::endl;
    }, "TIDTest");

    thread.start();

    // 等待线程启动并获取tid
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pid_t tid = thread.tid();
    assert(tid > 0);
    std::cout << "Thread TID from Thread object: " << tid << std::endl;

    thread.join();
    std::cout << "Thread ID test passed" << std::endl;
}

// 测试默认命名
void test_default_naming() {
    std::cout << "=== Test Default Naming ===" << std::endl;

    std::vector<std::unique_ptr<Thread>> threads;
    const int numThreads = 3;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(new Thread([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }));
    }

    for (int i = 0; i < numThreads; ++i) {
        threads[i]->start();
        std::cout << "Thread " << i << " name: " << threads[i]->name() << std::endl;
    }

    for (auto& t : threads) {
        t->join();
    }

    std::cout << "Default naming test passed" << std::endl;
}

// 测试多线程并发执行
void test_concurrent_threads() {
    std::cout << "=== Test Concurrent Threads ===" << std::endl;

    const int numThreads = 5;
    std::atomic<int> counter(0);
    std::vector<std::unique_ptr<Thread>> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(new Thread([&counter]() {
            counter.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }));
    }

    for (auto& t : threads) {
        t->start();
    }

    for (auto& t : threads) {
        t->join();
    }

    assert(counter == numThreads);
    std::cout << "Concurrent threads test passed" << std::endl;
}

// 测试线程计数
void test_thread_count() {
    std::cout << "=== Test Thread Count ===" << std::endl;

    int initialCount = Thread::numCreated();
    std::cout << "Initial thread count: " << initialCount << std::endl;

    {
        Thread t1([]() {});
        Thread t2([]() {});

        t1.start();
        t2.start();

        assert(Thread::numCreated() == initialCount + 2);

        t1.join();
        t2.join();
    }

    std::cout << "Final thread count: " << Thread::numCreated() << std::endl;
    std::cout << "Thread count test passed" << std::endl;
}

// 测试线程多次join
void test_multiple_join() {
    std::cout << "=== Test Multiple Join ===" << std::endl;

    Thread thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    thread.start();
    thread.join();

    // 第二次join不应该有副作用
    thread.join();

    std::cout << "Multiple join test passed" << std::endl;
}

int main() {
    std::cout << "=== Thread Tests ===" << std::endl;

    test_thread_creation();
    test_thread_id();
    test_default_naming();
    test_concurrent_threads();
    test_thread_count();
    test_multiple_join();

    std::cout << "=== All Thread Tests Passed ===" << std::endl;
    return 0;
}
