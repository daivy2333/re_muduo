#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

#include "noncopyable.h"


/**
 * @brief 线程封装类，提供基本的线程操作接口
 */
class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;

    /**
     * @brief 构造函数
     * @param func 线程执行函数
     * @param name 线程名称，默认为空字符串
     */
    explicit Thread(ThreadFunc func, const std::string& name = std::string());

    /**
     * @brief 析构函数，如果线程已启动但未join，则detach线程
     */
    ~Thread();

    /**
     * @brief 启动线程
     */
    void start();

    /**
     * @brief 等待线程结束
     */
    void join();

    /**
     * @brief 获取线程是否已启动
     * @return true 已启动，false 未启动
     */
    bool started() const { return m_started; }

    /**
     * @brief 获取线程ID
     * @return 线程ID
     */
    pid_t tid() const { return m_tid; }

    /**
     * @brief 获取线程名称
     * @return 线程名称的常量引用
     */
    const std::string& name() const { return m_name; }

    /**
     * @brief 获取已创建的线程总数
     * @return 已创建的线程数量
     */
    static int numCreated() { return m_numCreated; }

private:
    /**
     * @brief 设置默认线程名称
     */
    void setDefaultName();

    bool m_started;          ///< 线程是否已启动
    bool m_joined;           ///< 线程是否已join

    std::shared_ptr<std::thread> m_thread;  ///< 线程对象指针
    pid_t m_tid;             ///< 线程ID
    ThreadFunc m_func;       ///< 线程执行函数
    std::string m_name;      ///< 线程名称

    static std::atomic<int32_t> m_numCreated;  ///< 已创建的线程总数
};
