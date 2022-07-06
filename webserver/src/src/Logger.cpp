#include "Logger.h"
#include <iostream>
#include <sys/time.h>

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;
std::string Logger::LogFileName_ = "./WebServer.log";

// 线程函数
void once_init() {
    AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
    AsyncLogger_->start();  // 启动异步日志
}

void output(const char * msg, int len) {
    pthread_once(&once_control_, once_init);    // 一次性初始化
    AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char * fileName, int line) 
    : stream_(),
      line_(line),
      basename_(fileName)
{
    formatTime();
}

// 格式化当前时间，并写入LogStream
void Logger::Impl::formatTime() {
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream_ << str_t;
}

Logger::Logger(const char * fileName, int line)
    : impl_(fileName, line)
{   }

Logger::~Logger() {
    impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << "\n";
    const LogStream::Buffer & buf(stream().buffer());   // 获取缓冲区
    // std::cout << "~Logger() 创建Buffer:" << buf.getdata() << std::endl;
    output(buf.getdata(), buf.length());                // 默认输出到stdout
}