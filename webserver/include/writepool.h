#ifndef writepool_H
#define writepool_H

#include <pthread.h>
#include <list>
#include <exception>
#include <cstdio>
#include "safequeue.h"
#include <memory>
#include <semaphore.h>

// 线程池类，定义成模板类是为了代码复用,模板参数T就是任务类
template<typename T>
class writepool
{
private:
    // 线程数量
    int m_thread_num;

    // 线程池数组
    pthread_t *m_threads;

    // 请求队列最大允许的等待处理的请求数量
    int m_max_num;

    // 任务队列
    safequeue<T> m_workqueue;

    // 信号量用来判断是否有任务需要处理
    // sem m_queuestat;
    sem_t m_queuestat;

    // 是否结束线程
    bool m_stop;
    
    static void * worker(void * arg);

public:
    writepool(int num = 8, int max_requeuests = 10000);
    ~writepool();

    // 向任务队列提交任务
    bool submit(std::shared_ptr<T> request);

    // 启动线程池
    void run();
};

template<typename T>
writepool<T>::writepool(int num, int max_requeuests) : m_thread_num(num), m_max_num(max_requeuests),
    m_stop(false), m_threads(NULL) {
    if (num <= 0 || max_requeuests <= 0) {
        throw std::exception();
    }

    sem_init(&m_queuestat, 0, 0);
    // 初始化线程组,并将所有线程设置为线程分离
    m_threads = new pthread_t[num];
    if (!m_threads) {
        throw std::exception();
    }
    for (int i = 0; i < num; ++i) {
        printf("create the %dth thread\n", i);
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i])) {
            delete [] m_threads;
            throw std::exception();
        }
    }

}

template<typename T>
writepool<T>::~writepool()
{
    delete [] m_threads;
    m_stop = true;     // 线程判断自己是否要停止
}

template<typename T>
bool writepool<T>::submit(std::shared_ptr<T> request)
{
    // 如果任务量超出最大值，解锁并返回false
    if (m_workqueue.size() >= m_max_num) {
        return false;
    }

    // 向任务队列添加任务
    m_workqueue.enqueue(request);
    sem_post(&m_queuestat);
    // 唤醒阻塞的线程
    
    // printf("唤醒线程池线程\n");
    return true;
}

template<typename T>
void * writepool<T>::worker(void * arg)
{
    writepool * pool = (writepool *)arg;
    pool->run();
    return pool;
}

template<typename T>
void writepool<T>::run()
{
    while (!m_stop) {
        // 等待任务队列有待执行任务
        sem_wait(&m_queuestat);
        
        if (m_workqueue.empty()) {
            continue;
        }
        
        // 获取任务
        std::shared_ptr<T> task = std::make_shared<T> ();
        if (!m_workqueue.dequeue(task)) {
            continue;
        } 
            
        // printf("线程获取任务\n");
        // 任务执行
        if (!task->write()) {
            printf("写出错\n");
            task->close_conn();
        }
    }
}

#endif
