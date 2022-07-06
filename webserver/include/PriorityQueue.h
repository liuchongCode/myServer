#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <stdio.h>
#include <vector>
#include "http_connect.h"
#include <unordered_map>


struct client
{
    bool deleted_;
    int fd_;
    std::shared_ptr<http_connect> user_;
    size_t expiredTime_;
    client() : deleted_(false), expiredTime_(-1) {}
};

class PriorityQueue
{
private:
    std::unordered_map<int, size_t> map_;   // fd映射到heap_中的的索引
    std::vector<client> heap_;
    int size_;
public:
    PriorityQueue() : size_(0) {}
    ~PriorityQueue() {
    }

    void add(std::shared_ptr<http_connect>, int timeout);
    void update(int index, int val);
    int top();
    void pop();
    int UpToDown(int i, int len);
    int DownToUp(int len);
    void swap(int x, int y);
    void handleExpiredClient();
    bool setDeleted(int fd);
    int getnexttick();
    void show() {
        size_t tmp = heap_[0].expiredTime_;
        for (int i = 0; i < size_; ++i) {
            printf("index = %d, fd = %d, time = %ld\n", i, heap_[i].fd_, heap_[i].expiredTime_ - tmp);
        }
    }
};

#endif
