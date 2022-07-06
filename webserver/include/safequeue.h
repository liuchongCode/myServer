#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <queue>
#include <mutex>
#include <memory>
#define QUEUESIZE 65535

// 在std::queue基础上封装一个线程安全的队列
template <typename T>
class safequeue
{
private:
    T m_queue[QUEUESIZE];  // 利用模板函数构造队列
    std::mutex m_mutex;  // 访问互斥信号量
    int m_number; // 队列任务数量
    int m_head;  // 队头
    int m_tail;  // 队尾
    int m_size;   // 队列总长度
public:
    safequeue() : m_size(QUEUESIZE), m_number(0), m_head(0), m_tail(0) {
    }

    safequeue(safequeue & other) {}
    ~safequeue() {}

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex); // 互斥信号量加锁，防止m_queue被改变
        return m_number <= 0;
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_number;
    }

    // 添加元素
    void enqueue(T t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_number >= m_size)
            return;
        m_queue[m_tail] = t;
        m_tail = (m_tail + 1) % m_size;
        m_number++;
    }

    // 取元素
    bool dequeue(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_number <= 0) {
            return false;
        }
        t = m_queue[m_head]; 
        m_head = (m_head + 1) % m_size;
        m_number--;
        return true;
    }  
};

#endif