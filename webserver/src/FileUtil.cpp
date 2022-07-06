#include "FileUtil.h"
#include <iostream>

AppendFile::AppendFile(std::string filename) : fp_(fopen(filename.c_str(), "ae")) {
    std::cout << "AppendFile() : 打开文件名:" << filename << std::endl;
    if (!fp_)
        std::cout << "打开文件失败\n";
    setbuffer(fp_, buffer_, sizeof buffer_);
    printf("打开文件，设置缓冲区\n");
}

AppendFile::~AppendFile() {
    fclose(fp_);
}

/* 向文件中写入长度为len的logline */
void AppendFile::append(const char * logline, const size_t len) {
    // 数据写入文件
    size_t n = this->write(logline, len);
    size_t remain = len - n;
    while (remain > 0) {
        size_t bytes = this->write(logline + n, len);
        // 写入失败
        if (bytes == 0) {
            // 测试由fp_指向的流的错误指示器，如果设置了它，则返回非零。错误指示器只能通过clearerr()函数重置。
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "AppendFile::append() failed\n"); // 报错信息
            }
            break;
        }

        n += bytes;
        remain = len - n; // 剩余要写的数量
    }
}

void AppendFile::flush() {
    /* 将任何输出流 （或更新流）的内容刷新到相应文件的函数 */
    fflush(fp_); // 刷新缓冲区
    // std::cout << fp_->_fileno << std::endl;
}

size_t AppendFile::write(const char * logline, size_t len) {
    // 不加锁的方式写文件，线程不安全，但是效率高
    return fwrite_unlocked(logline, 1, len, fp_);
}
