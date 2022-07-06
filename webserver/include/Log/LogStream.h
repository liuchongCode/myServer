#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include "nocopyable.h"
#include <string.h>
#include <string>
#include <assert.h>
#include <memory>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>


const int kSmallBuffer = 4000;
const int klargeBuffer = 4000 * 1000;

/*
    该文件主要用来格式化输出, 重载 << 运算符
*/

// 非类型参数模板
template<int SIZE>
class FixedBuffer: nocopyable   // 定义一个缓冲区类
{
private:
    const char * end() const {          // 返回缓冲区的末端地址
        return data + sizeof data;
    }

    char data[SIZE];
    char * cur_;

public:
    FixedBuffer() : cur_(data) {
        // printf("初始化Buffer\n");
    }
    ~FixedBuffer() {}

    const char * getdata() const {   // 缓冲区首地址
        return data;
    }

    int length() const  {         // 缓冲区偏移量
        return static_cast<int>(cur_ - data);
    }

    char * current() {          // 返回当前数据末端地址
        return cur_;
    }

    int avail() const { return static_cast<int> (end() - cur_); }  // 返回剩余可用地址
    void add(size_t len) { cur_ += len; }   // 偏移量后移

    void bzero() {  
        memset(data, 0, sizeof data);
    }    // 清空数据    
    void reset() {  cur_ = data; }                      // 重置，但不清除数据，即偏移量重置

    void append(const char * buf, size_t len) {
        if (avail() > static_cast<int>(len)) {      // 如果可用空间充足，就拷贝
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

};


// 重载 << 运算符
class LogStream : nocopyable
{
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;
private:
    void staticCheck();

    template<typename T>
    void formatInteger(T v);  

    Buffer buffer_;                         // 缓冲区

    static const int kMaxNumericSize = 32;  // 表示数值的最大字节数
public:
    LogStream& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);
    LogStream& operator<<(long double);

    LogStream& operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }

    LogStream& operator<<(const char* str)
    {
        if (str)
            buffer_.append(str, strlen(str));
        else
            buffer_.append("(null)", 6);
        return *this;
    }

    LogStream& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    void append(const char * data, int len) { buffer_.append(data, len); }  // 向缓冲区添加数据
    const Buffer & buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); } // 重置当前缓冲区，但不清除数据
};

#endif

