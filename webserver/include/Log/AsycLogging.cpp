#include "AsyncLogging.h"

AsyncLogging::AsyncLogging(std::string logFileName_, int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(logFileName_),
      thread_(new ThreadPool(1)),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_()
{
    assert(logFileName_.size() > 1);
    // 初始化缓冲区
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

// 发送方
void AsyncLogging::append(const char * logline, int len) {
    mutex_.lock();
    // 通常情况下，缓冲区并不满，将数据拷贝到缓冲区
    if (currentBuffer_->avail() > len) {
        currentBuffer_->append(logline, len);
        mutex_.unlock();
    } 
    else // 缓冲区满时，放入已填满缓冲区数组，并找到下一个备用缓冲区
    {
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset();
        if (nextBuffer_)
            currentBuffer_ = std::move(nextBuffer_);    // 如果有备用缓冲区，移动，而不是复制
        else    
            currentBuffer_.reset(new Buffer);   // 如果没有就申请一个新的
        currentBuffer_->append(logline, len);
        cond_.signal();     // 唤醒后端开始写入日志
        mutex_.unlock();
    }
}

// 后端接收方线程调用该函数，周期性的flush数据到日志文件中
void AsyncLogging::threadFunc() {
    if (running_ != true)
        return;
    printf("线程启动\n");
    LogFile output(basename_);
    BufferPtr newBuffer1(new Buffer);       // 两块空闲缓冲区，以备在临界区内交换
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();    // 初始化
    newBuffer2->bzero();
    BufferVector buffersToWrite;    // 写入文件
    buffersToWrite.reserve(16);
    while (running_) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        // 将要写入的数据（buffers_中的数据），swap到缓冲区buffersToWrite中
        {
            mutex_.lock();
            // 如果buffer为空，那么表示没有数据需要写入文件，就指定等待时间
            if (buffers_.empty()) {
                printf("缓冲区为空，等待数据写入\n");
                cond_.timewait(flushInterval_);
            }

            printf("唤醒\n");
            // 无论 cond_ 为什么被唤醒，都要将currentBuffer_ 放入 buffers_中
            buffers_.push_back(currentBuffer_);     // 移动
            currentBuffer_.reset();
            currentBuffer_ = std::move(newBuffer1); // 移动
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_) {
                nextBuffer_ = std::move(newBuffer2);    // 内部指针交换
            }
        }

        assert(!buffersToWrite.empty());

        // 如果将要写入的buffer 个数大于25，将多余的数据删除，消息堆积
        if (buffersToWrite.size() > 25) {
            // 丢掉多余日志，腾出内存，仅保留两块缓冲区
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        // 将buffersToWrite 中的数据写入文
        for (size_t i = 0; i < buffersToWrite.size(); ++i) {
            output.append(buffersToWrite[i]->getdata(), buffersToWrite[i]->length());
        }

        // 仅保留两个buffer
        if (buffersToWrite.size() > 2) {
            // 删除非空缓冲区，避免浪费
            buffersToWrite.resize(2);
        }

        // 重置 newBuffer1 and newBuffer2
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        // 清空
        buffersToWrite.clear();
        output.flush();
    }
    // 结束后仍要将剩余数据写入文件
    output.flush();

}