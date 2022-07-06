#ifndef LOGGER_H
#define LOGGER_H

#include "LogStream.h"
#include "AsyncLogging.h"

// 对外提供日志接口
class Logger
{
private:
    class Impl
    {
    public:
        Impl(const char * filename, int line);
        void formatTime();      // 格式化时间

        LogStream stream_;
        int line_;              // 行号
        std::string basename_;
    };

    Impl impl_;
    static std::string LogFileName_;
    
public:
    Logger(const char * fileName, int line);
    ~Logger();

    LogStream& stream() { return impl_.stream_; } // 返回一个LogStream对象

    static void setLogFileName(std::string fileName) {
        LogFileName_ = fileName;
    }

    static std::string getLogFileName() {
        return LogFileName_;
    }
};

#define LOG Logger(__FILE__, __LINE__).stream()

#endif