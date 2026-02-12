#include "Logger.h"
#include "AsyncLogging.h"
#include <stdio.h>
#include <string.h>

std::function<void(const char*, int)> Logger::output_ = [](const char* msg, int len) {
    fwrite(msg, 1, len, stdout);
};

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level) {
    LogLevel_ = level;
}

void Logger::setOutput(std::function<void(const char*, int)> output) {
    output_ = output;
}

std::function<void(const char*, int)>& Logger::output() {
    return output_;
}

void Logger::log(std::string msg)
{
    const char* levelStr = "";
    switch (LogLevel_)
    {
    case INFO:
        levelStr = "INFO";
        break;
    case ERROR:
        levelStr = "ERROR";
        break;
    case FATAL:
        levelStr = "FATAL";
        break;
    case DEBUG:
        levelStr = "DEBUG";
        break;
    default:
        levelStr = "UNKNOWN";
        break;
    }

    char buf[1024];
    int len = snprintf(buf, sizeof(buf), "[%s] %s: %s\n", 
                       Timestamp::now().to_string().c_str(), levelStr, msg.c_str());
    output_(buf, len);
}
