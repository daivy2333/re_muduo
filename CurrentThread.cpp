#include "CurrentThread.h"
#include <sys/syscall.h>
#include <unistd.h>

namespace CurrentThread {

// 线程局部存储的线程ID初始化
__thread int t_cachedTid = 0;

void cacheTid() {
    if (t_cachedTid == 0) {
        // 通过系统调用获取真实的线程ID
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}

} // namespace CurrentThread
