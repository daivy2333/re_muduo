#pragma once

#include <iostream>
#include <string>
#include <functional>

#include "noncopyable.h"
#include "LogStream.h"
#include "Timestamp.h"

// 定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel {
    INFO,   ///< 信息级别
    ERROR,  ///< 错误级别
    FATAL,  ///< 致命错误级别
    DEBUG,  ///< 调试级别
};

/**
 * @brief Logger类，日志记录器
 *
 * 提供了日志记录功能，支持不同级别的日志输出
 */
class Logger : noncopyable {
public:
    /**
     * @brief 获取Logger单例实例
     * @return Logger单例的引用
     */
    static Logger& instance();

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLogLevel(int level);

    /**
     * @brief 记录日志
     * @param msg 日志消息
     */
    void log(std::string msg);

    /**
     * @brief 设置日志输出回调
     * @param output 输出回调函数
     */
    static void setOutput(std::function<void(const char*, int)> output);

    /**
     * @brief 获取日志输出回调
     * @return 输出回调函数
     */
    static std::function<void(const char*, int)>& output();

private:
    int LogLevel_;  ///< 当前日志级别
    Logger() {};   ///< 私有构造函数，实现单例模式

    static std::function<void(const char*, int)> output_;
};

// log的宏
#define LOG_INFO(LogmsgFormat, ...) \
    do \
    {\
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0)
#define LOG_ERROR(LogmsgFormat, ...) \
    do \
    {\
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0)
#define LOG_FATAL(LogmsgFormat, ...) \
    do \
    {\
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
        exit(-1); \
    }while(0)
#ifdef MUDEBUF
#define LOG_DEBUG(LogmsgFormat, ...) \
    do \
    {\
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while(0)
#else
    #define LOG_DEBUG(LogmsgFormat, ...)
#endif
