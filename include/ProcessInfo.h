#pragma once

#include <string>
#include <vector>
#include <sys/types.h>

namespace ProcessInfo {
    pid_t pid();
    std::string pidString();
    uid_t uid();
    std::string username();
    uid_t euid();
    std::string hostname();
    std::string procname();
    std::string procStatus();
    int openFiles();
    int maxOpenFiles();
    int threads();
    std::vector<pid_t> threadsList();
}
