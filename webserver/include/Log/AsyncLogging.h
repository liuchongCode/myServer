#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H

#include "LogStream.h"
#include "LogFile.h"
#include <vector>
#include <functional>
#include "ThreadPool.h"

// 启动一个log线程，专门用来将log写入LogFile，应用了“双缓冲技术”，实际上是4块缓冲区
// 定时或者被填满时，将缓冲区中的数据写入LogFile
class AsyncLogging : nocopyable
{
private:
    void threadFunc();
    typedef FixedBuffer<klargeBuffer> Buffer;
    typedef std::vector<std::shared_ptr<Buffer> > BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;                  // 自动管理对象声明周期
    const int flushInterval_;                                   // 超时时间
    bool running_;                                              // 是否运行标志
    std::string basename_;          
    std::unique_ptr<ThreadPool> thread_;                        // 执行该异步日志的线程
    locker mutex_;
    cond cond_;
    BufferPtr currentBuffer_;                                   // 当前缓冲区
    BufferPtr nextBuffer_;                                      // 预备缓冲区
    BufferVector buffers_;                                      // 待写入文件的已填满的缓冲区
public:
    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging() {
        if (running_)
            stop();
    }

    void append(const char * logline, int len);
    void start() {
        if (!running_) {
            running_ = true;
            thread_->submit(std::bind(&AsyncLogging::threadFunc, this));
        }          
    }

    void stop() {
        running_ = false;
        cond_.signal();
        // thread_.get();
    }
};

#endif