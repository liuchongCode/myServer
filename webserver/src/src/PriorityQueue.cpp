#include "PriorityQueue.h"
#include <sys/time.h>

void PriorityQueue::swap(int a, int b) {
    client c = heap_[b];
    heap_[b] = heap_[a];
    heap_[a] = c;
    map_[heap_[b].fd_] = b;
    map_[heap_[a].fd_] = a;
}

void PriorityQueue::add(std::shared_ptr<http_connect> user, int timeout)
{
    // 如果该连接已经存在，则更新该连接
    int fd = user->getsockfd();
    if (map_.count(fd)) {
        // printf("有这个元素 %d\n", fd);
        update(fd, timeout);
        return;
    }
    // 否则添加
    struct timeval now;
    gettimeofday(&now, NULL);
    client node;
    node.expiredTime_ = (long)(now.tv_sec * 1000) + now.tv_usec / 1000 + timeout;    // 当前时间加上定时时间
    node.user_ = user;
    node.fd_ = fd;
    heap_.push_back(std::move(node));
    map_[fd] = size_;
    size_++;
    DownToUp(size_ - 1);
}

void PriorityQueue::update(int fd, int timeout)
{
    int index = map_[fd];
    struct timeval now;
    gettimeofday(&now, NULL);
    heap_[index].deleted_ = false;
    heap_[index].expiredTime_ = (long)(now.tv_sec * 1000) + now.tv_usec / 1000 + timeout;
    UpToDown(index, size_);
}

int PriorityQueue::top() {
    if (size_ > 0 && !heap_[0].deleted_) {
        return heap_[0].expiredTime_;
    }
    return 0;
}

void PriorityQueue::pop() {
    // printf("index = %d, fd = %d, time = %ld, size = %d\n", 0, heap_[0].fd_, heap_[0].expiredTime_, size_);
    --size_;
    map_.erase(heap_[0].fd_);
    if (size_ > 0) {
        map_[heap_[size_].fd_] = 0;
        // printf("%ld\n", heap_[0].user_.use_count());
        heap_[0] = heap_[size_];
    }
    heap_.pop_back();
    UpToDown(0, size_);
}

// 更新或删除元素时，自上而下调整
int PriorityQueue::UpToDown(int i, int len)
{
    for(; (i << 1) + 1 < len; ) {
        int left = (i << 1) + 1;       //由于编号要与数组下标对应，因此从0开始，乘2加1为左孩子
        int right = (i << 1) + 2;
        int large = i;
        if(left < len && heap_[left].expiredTime_ < heap_[i].expiredTime_) {
            large = left;
        }
        if(right < len && heap_[right].expiredTime_ < heap_[large].expiredTime_) {
            large = right;
        }
        if(large != i) {        //如果父节点不是最大的，则将三者最大的最为父节点
            swap(i, large);
            i = large;          //以此继续向下调整
        } else {
            break;              //如果父节点就是最大的，则本子树不需要再调整
        }
    }
    return i;
}

// 增加元素时，自下而上调整
int PriorityQueue::DownToUp(int i)
{
    if (i <= 0)
        return i;
    for(; ((i - 1) >> 1) >= 0; ) {
        int parent = ((i - 1) >> 1);       //由于编号要与数组下标对应，因此从0开始，乘2加1为左孩子
        if (parent >= 0 && heap_[parent].expiredTime_ > heap_[i].expiredTime_) {
            swap(i, parent);
            i = parent;
        } else {
            break;
        }
    }
    return i;
}

// 不在堆顶但要删除的节点设置为已删除，但并没有实际删除
bool PriorityQueue::setDeleted(int fd) {
    if (map_.count(fd)) {
        heap_[map_[fd]].deleted_ = true;
        return true;
    } else {
        return false;
    }
}

void PriorityQueue::handleExpiredClient() {
    struct timeval now;
    gettimeofday(&now, NULL);
    int curr = (long)(now.tv_sec * 1000) + now.tv_usec / 1000;
    while (size_ > 0 && (curr > top() || heap_[0].deleted_)) {
        heap_[0].user_->close_conn();
        pop();
    }
}

int PriorityQueue::getnexttick() {
    if (size_ == 0)
        return -1;
    int next = top();
    if (next == 0)
        return 0;
    struct timeval now;
    gettimeofday(&now, NULL);
    int curr = (long)(now.tv_sec * 1000) + now.tv_usec / 1000;
    int ret = top() - curr;
    if (ret <= 0) {
        handleExpiredClient();
        if (size_ == 0)
            return -1;
        ret = top() - curr;
    }
    return ret;
}