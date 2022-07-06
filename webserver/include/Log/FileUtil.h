#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string>
#include "nocopyable.h"
#include <fcntl.h>
// #include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

/* 封装Log日志系统中所使用的文件的打开、写入，并在析构类的时候关闭文件 */

class AppendFile : nocopyable
{
private:
    FILE* fp_; // 对应的文件
    size_t write(const char * logline, size_t len);
    char buffer_[64*1024]; // 64k的缓冲区
public:
    explicit AppendFile(std::string filename);  // explicit禁止隐式类型转换
    ~AppendFile();

    // append 中会向文件写内容
    void append(const char * logline, const size_t len);
    void flush();
};

#endif