#include "AsyncLoggingInit.h"
#include "Logger.h"
#include <memory>
#include <cstdlib>

namespace {
std::unique_ptr<AsyncLogging> g_asyncLog;
bool g_testMode = false;
bool g_hasErrorLog = false;
}

void initAsyncLogging(const std::string& basename, off_t rollSize, int flushInterval) {
    if (!g_asyncLog) {
        g_asyncLog.reset(new AsyncLogging(basename, rollSize, flushInterval));
        g_asyncLog->start();
    }
}

void resetAsyncLogging() {
    if (g_asyncLog) {
        g_asyncLog->stop();
        g_asyncLog.reset();
    }
}

void setAsyncOutput() {
    if (g_asyncLog) {
        Logger::setOutput([](const char* msg, int len) {
            // 检查是否是错误日志，错误日志以"E"开头
            if (len > 0 && msg[0] == 'E') {
                g_hasErrorLog = true;
            }
            g_asyncLog->append(msg, len);
        });
    }
}

void enableTestMode() {
    g_testMode = true;
    g_hasErrorLog = false;
}

bool hasErrorLog() {
    return g_hasErrorLog;
}

void cleanupLogFiles() {
    system("find log -type f -name '*.log' -delete 2>/dev/null");
    system("find log -type d -empty -delete 2>/dev/null");
}
