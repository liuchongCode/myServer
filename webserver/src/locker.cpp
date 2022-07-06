#include "locker.h"


locker::locker()
{
    if (pthread_mutex_init(&my_mutex, NULL) != 0) {
        throw std::exception();                   // 如果初始化失败抛出异常
    }
}

locker::~locker()
{
    pthread_mutex_destroy(&my_mutex);
}


cond::cond(locker &mutex_) : mutex(mutex_)
{
    if (pthread_cond_init(&my_cond, NULL) != 0) {
        throw std::exception();
    }
}

cond::~cond()
{
    pthread_cond_destroy(&my_cond);
}


sem::sem()
{
    if(sem_init(&my_sem, 0, 0) != 0) {
        throw std::exception();
    }
}

sem::sem(int num)
{
    if(sem_init(&my_sem, 0, num) != 0) {
        throw std::exception();
    }
}

sem::~sem()
{
    sem_destroy(&my_sem);
}
