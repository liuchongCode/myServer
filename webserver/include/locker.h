#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

// 线程同步机制封装类

// 1、互斥锁类
class locker
{
private:
    pthread_mutex_t my_mutex;
public:
    locker();
    ~locker();

    // 加锁
    bool lock() {
        return pthread_mutex_lock(&my_mutex) == 0;
    }

    // 解锁
    bool unlock() {
        return pthread_mutex_unlock(&my_mutex) == 0;
    }

    // 获取锁
    pthread_mutex_t * getmutex() {
        return &my_mutex;
    }
};

// 2、条件变量类
class cond
{
private:
    pthread_cond_t my_cond;
    locker & mutex;
public:
    explicit cond(locker &mutex_);
    ~cond();

    // 等待，需要配合互斥量使用
    bool wait() {
        return pthread_cond_wait(&my_cond, mutex.getmutex()) == 0;
    }

    // 带超时时间等待，需要配合互斥量使用
    bool timewait(struct timespec t) {
        return pthread_cond_timedwait(&my_cond, mutex.getmutex(), &t) == 0;
    }

    // 带超时时间等待，需要配合互斥量使用
    bool timewait(int t) {
        struct timespec _t;
        clock_gettime(CLOCK_REALTIME, &_t);                // 标准POSIX 实时时钟， 纳秒
        _t.tv_sec += static_cast<time_t>(t);
        return ETIMEDOUT ==  pthread_cond_timedwait(&my_cond, mutex.getmutex(), &_t);
    }

    // V操作，唤醒一个或多个线程，需要配合互斥量使用
    bool signal() {
        return pthread_cond_signal(&my_cond) == 0;
    }

    // V操作，唤醒所有线程，需要配合互斥量使用
    bool broadcast() {
        return pthread_cond_broadcast(&my_cond) == 0;
    }
};



// 信号量类
class sem
{
private:
    sem_t my_sem;
public:
    sem();
    sem(int num);
    ~sem();

    // 等待信号量
    bool wait() {
        return sem_wait(&my_sem) == 0;
    }

    // 增加信号量
    bool post() {
        return sem_post(&my_sem) == 0;
    }
};


#endif



