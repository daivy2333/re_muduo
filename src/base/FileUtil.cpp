#include "FileUtil.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

using namespace std;

AppendFile::AppendFile(string filename)
    : fp_(::fopen(filename.data(), "ae")),
      writtenBytes_(0) {
    if (!fp_) {
        // 尝试创建目录
        char filename_copy[filename.size() + 1];
        strcpy(filename_copy, filename.c_str());
        char* dir = dirname(filename_copy);
        if (mkdir(dir, 0755) == 0 || errno == EEXIST) {
            // 目录创建成功或已存在，再次尝试打开文件
            fp_ = ::fopen(filename.data(), "ae");
        }

        if (!fp_) {
            fprintf(stderr, "AppendFile::AppendFile() failed to open file %s: %s\n", 
                    filename.c_str(), strerror(errno));
        }
    }

    if (fp_) {
        ::setbuffer(fp_, buffer_, sizeof(buffer_));
    }
}

AppendFile::~AppendFile() {
    if (fp_) {
        ::fclose(fp_);
    }
}

void AppendFile::append(const char* logline, size_t len) {
    if (!fp_) {
        return;
    }

    size_t n = write(logline, len);
    size_t remain = len - n;
    while (remain > 0) {
        size_t x = write(logline + n, remain);
        if (x == 0) {
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
            }
            break;
        }
        n += x;
        remain = len - n;
    }
    writtenBytes_ += len;
}

void AppendFile::flush() {
    if (fp_) {
        ::fflush(fp_);
    }
}

size_t AppendFile::write(const char* logline, size_t len) {
    return fwrite_unlocked(logline, 1, len, fp_);
}
