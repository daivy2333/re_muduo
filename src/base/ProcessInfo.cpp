#include "ProcessInfo.h"
#include "CurrentThread.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <pwd.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace ProcessInfo {
pid_t pid() {
    return ::getpid();
}

std::string pidString() {
    char buf[32];
    snprintf(buf, sizeof buf, "%d", pid());
    return buf;
}

uid_t uid() {
    return ::getuid();
}

std::string username() {
    struct passwd pwd;
    struct passwd* result = NULL;
    char buf[8192];
    getpwuid_r(uid(), &pwd, buf, sizeof buf, &result);
    return result ? pwd.pw_name : std::string();
}

uid_t euid() {
    return ::geteuid();
}

std::string hostname() {
    char buf[256];
    if (::gethostname(buf, sizeof buf) == 0) {
        buf[sizeof buf - 1] = '\0';
        return buf;
    } else {
        return "unknownhost";
    }
}

std::string procname() {
    return "unknown";
}

std::string procStatus() {
    char buf[1024];
    snprintf(buf, sizeof buf, "/proc/%d/status", pid());
    FILE* fp = fopen(buf, "r");
    if (fp) {
        std::string content;
        char line[256];
        while (fgets(line, sizeof line, fp)) {
            content += line;
        }
        fclose(fp);
        return content;
    }
    return std::string();
}

int openFiles() {
    int count = 0;
    char buf[64];
    snprintf(buf, sizeof buf, "/proc/%d/fd", pid());
    DIR* pdir = opendir(buf);
    if (pdir) {
        struct dirent* pentry;
        while ((pentry = readdir(pdir)) != NULL) {
            if (isdigit(pentry->d_name[0])) {
                ++count;
            }
        }
        closedir(pdir);
    }
    return count;
}

int maxOpenFiles() {
    struct rlimit limit;
    if (::getrlimit(RLIMIT_NOFILE, &limit) == 0) {
        return static_cast<int>(limit.rlim_cur);
    }
    return -1;
}

int threads() {
    int result = 0;
    char buf[64];
    snprintf(buf, sizeof buf, "/proc/%d/status", pid());
    FILE* fp = fopen(buf, "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof line, fp)) {
            if (strncmp(line, "Threads:", 8) == 0) {
                result = atoi(line + 8);
                break;
            }
        }
        fclose(fp);
    }
    return result;
}

std::vector<pid_t> threadsList() {
    std::vector<pid_t> result;
    char buf[64];
    snprintf(buf, sizeof buf, "/proc/%d/task", pid());
    DIR* pdir = opendir(buf);
    if (pdir) {
        struct dirent* pentry;
        while ((pentry = readdir(pdir)) != NULL) {
            if (isdigit(pentry->d_name[0])) {
                result.push_back(atoi(pentry->d_name));
            }
        }
        closedir(pdir);
    }
    return result;
}

}
