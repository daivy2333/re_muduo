#include "Logger.h"

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level) {
    LogLevel_ = level;
}


void Logger::log(std::string msg)
{
    switch (LogLevel_)
    {
    case INFO:
        std::cout << "INFO";
        break;
    case ERROR:
        std::cout << "E";
        break;
    
    case FATAL:
        std::cout << "F";
        break;
    case DEBUG:
        std::cout << "D";
        break;
    
    default:
        break;
    }
    std::cout<< "time!"<<": "<< msg<< std::endl;
}