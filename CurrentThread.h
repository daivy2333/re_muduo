#pragma once

namespace CurrentThread {

/**
 * @brief 线程局部存储的线程ID缓存
 * 
 * 使用 __thread 关键字实现线程局部存储，
 * 每个线程都有自己独立的 t_cachedTid 变量
 */
extern __thread int t_cachedTid;

/**
 * @brief 缓存当前线程的TID
 * 
 * 通过系统调用获取真实的线程ID并缓存到 t_cachedTid
 */
void cacheTid();

/**
 * @brief 获取当前线程的TID
 * 
 * @return 当前线程的线程ID
 * 
 * 使用 __builtin_expect 进行分支预测优化，
 * 预期 t_cachedTid 不为0（即已经缓存过）
 */
inline int tid() {
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

} // namespace CurrentThread