#ifndef LOGFILE_H
#define LOGFILE_H

#include "FileUtil.h"
#include "locker.h"
#include <memory>
#include <string>

/* 
    提供自动刷新缓冲区功能: 每写入N行刷新一次缓冲区
*/
class LogFile : nocopyable
{
private:
    void append_unlocked(const char * logline, int len);    // 不加锁的append方式
    const std::string basename_;                            // 日志文件basename
    const int flushEveryN_; 
    int count_;                                             // 计数器，检测是否需要刷新文件
    std::unique_ptr<locker> m_mutex;                        // 锁
    std::unique_ptr<AppendFile> file_;                      // 文件智能指针
public:
    // 每次append flushEverN次，flush一次，会往文件中写，只不过封装过的AppendFile文件自带缓冲区
    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile();

    void append(const char * logline, int len);
    void flush();
    // bool rollfile();
};

#endif
