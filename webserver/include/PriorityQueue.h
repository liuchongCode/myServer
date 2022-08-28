#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <stdio.h>
#include <vector>
#include "http_connect.h"
#include <unordered_map>
#include "Log/Logger.h"


struct client
{
    bool deleted_;
    int index_;
    int fd_;
    std::shared_ptr<http_connect> user_;
    size_t expiredTime_;
    client() : index_(-1), deleted_(false), expiredTime_(-1) {}
};

class PriorityQueue
{
private:
    // std::unordered_map<int, size_t> map_;   // fd映射到heap_中的的索引
    std::vector<client> heap_;
    int size_;
    std::mutex mutex_;
public:
    PriorityQueue() : size_(0) {}
    ~PriorityQueue() {
    }
    bool isContain(const std::shared_ptr<http_connect>& user) {
        if (!user)
            return false;
        std::unique_lock<std::mutex> lock(mutex_);
        return user->timer != nullptr;
    }
    size_t top();
    void pop();
    int UpToDown(int i, int len);
    int DownToUp(int len);
    void swap(int x, int y);
    void add(std::shared_ptr<http_connect>& user, int timeout);
    void update(const std::shared_ptr<http_connect>& user, int val);
    bool setDeleted(const std::shared_ptr<http_connect>& user);
    void handleExpiredClient();
    int getnexttick();
    void show() {
        size_t tmp = heap_[0].expiredTime_;
        for (int i = 0; i < size_; ++i) {
            printf("index = %d, fd = %d, time = %ld\n", i, heap_[i].fd_, heap_[i].expiredTime_ - tmp);
        }
    }
};

#endif
