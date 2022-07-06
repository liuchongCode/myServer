#include "LogFile.h"
#include <iostream>

LogFile::LogFile(const std::string& basename, int flushEveryN)
    : basename_(basename),
      flushEveryN_(flushEveryN),
      count_(0),
      m_mutex(new locker) // 默认加锁
{
    // std::cout << "LogFile() : 初始化成员变量（即文件指针）前" << std::endl;
    // 初始化文件智能指针
    file_.reset(new AppendFile(basename));
    // std::cout << "LogFile() : 初始化成员变量（即文件指针）" << std::endl;
}

LogFile::~LogFile()
{
}

// 加锁的方式追加日志文本
void LogFile::append(const char * logline, int len) {
    m_mutex->lock();
    append_unlocked(logline, len);
    m_mutex->unlock();
}

void LogFile::append_unlocked(const char * logline, int len) {
    this->file_->append(logline, len); // 写入一行
    ++count_;    // 增加行数
    printf("LogFile append(): count = %d\n", count_);
    if (count_ >= flushEveryN_) {     // 每n行刷新一次缓冲区，即，将缓冲区内容写入文件
        count_ = 0;
        file_->flush();
    }
}

// 加锁的方式刷新缓冲区
void LogFile::flush() {
    m_mutex->lock();
    file_->flush();
    m_mutex->unlock();
}
