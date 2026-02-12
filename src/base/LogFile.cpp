#include "LogFile.h"
#include "FileUtil.h"
#include "ProcessInfo.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

using namespace std;

LogFile::LogFile(const string& basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
    : basename_(basename),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      checkEveryN_(checkEveryN),
      count_(0),
      mutex_(threadSafe ? new MutexLock : NULL),
      startOfPeriod_(0),
      lastRoll_(0),
      file_(nullptr) {
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len) {
    if (mutex_) {
        MutexLockGuard lock(*mutex_);
        append_unlocked(logline, len);
    } else {
        append_unlocked(logline, len);
    }
}

void LogFile::flush() {
    if (mutex_) {
        MutexLockGuard lock(*mutex_);
        file_->flush();
    } else {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len) {
    file_->append(logline, len);
    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    } else {
        ++count_;
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds * kRollPerSeconds;
            if (thisPeriod_ != startOfPeriod_) {
                rollFile();
            } else if (now - lastRoll_ > flushInterval_) {
                lastRoll_ = now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds * kRollPerSeconds;

    // 从完整路径中提取目录部分
    size_t pos = filename.find_last_of('/');
    if (pos != string::npos) {
        string dirname = filename.substr(0, pos);
        // 确保目录存在
        string mkdirCmd = "mkdir -p " + dirname;
        system(mkdirCmd.c_str());
    }

    // 总是创建新的日志文件，即使时间戳相同
    lastRoll_ = now;
    startOfPeriod_ = start;
    file_.reset(new AppendFile(filename));
    return true;
}

string LogFile::getLogFileName(const string& basename, time_t* now) {
    string filename;
    filename.reserve(basename.size() + 64);
    
    // 直接使用basename，不添加硬编码的前缀
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    // 添加微秒级时间戳以确保文件名唯一
    struct timeval tv;
    gettimeofday(&tv, NULL);
    char usecbuf[16];
    snprintf(usecbuf, sizeof(usecbuf), ".%06ld", tv.tv_usec);
    filename += usecbuf;

    filename += ".log";

    return filename;
}
