#pragma once

#include <string>
#include <functional>
#include "AsyncLogging.h"

/**
 * @brief 初始化异步日志系统
 *
 * @param basename 日志文件名前缀
 * @param rollSize 日志文件滚动大小（字节）
 * @param flushInterval 刷新间隔（秒）
 */
void initAsyncLogging(const std::string& basename,
                      off_t rollSize = 1024 * 1024 * 1024,  // 1GB
                      int flushInterval = 3);

/**
 * @brief 重置异步日志系统
 *
 * 停止当前的异步日志系统，允许重新初始化
 */
void resetAsyncLogging();

/**
 * @brief 设置异步日志输出
 *
 * 将Logger的输出重定向到异步日志系统
 */
void setAsyncOutput();

/**
 * @brief 启用测试模式
 *
 * 在测试模式下，如果没有错误日志，程序退出时会自动删除所有日志文件
 */
void enableTestMode();

/**
 * @brief 检查是否有错误日志
 *
 * @return true 如果有错误日志
 * @return false 如果没有错误日志
 */
bool hasErrorLog();

/**
 * @brief 清理所有日志文件
 *
 * 删除所有日志文件
 */
void cleanupLogFiles();
