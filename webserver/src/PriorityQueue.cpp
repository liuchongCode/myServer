#include "PriorityQueue.h"
#include <sys/time.h>

void PriorityQueue::swap(int a, int b) {
    client c = heap_[b];
    heap_[b] = heap_[a];
    heap_[a] = c;
    int tmp = heap_[a].index_;
    heap_[a].index_ = heap_[b].index_;
    heap_[b].index_ = tmp;
}

void PriorityQueue::add(std::shared_ptr<http_connect>& user, int timeout)
{
    int fd = user->getsockfd();
    std::unique_lock<std::mutex> lock(mutex_);
    struct timeval now;
    gettimeofday(&now, NULL);
    client node;
    node.expiredTime_ = (long)(now.tv_sec * 1000) + now.tv_usec / 1000 + timeout;    // 当前时间加上定时时间
    node.user_ = user;
    node.fd_ = fd;
    node.index_ = size_;
    user->timer = std::make_shared<client> (node);
    size_++;
    heap_.push_back(std::move(node));
    DownToUp(size_ - 1);
}

void PriorityQueue::update(const std::shared_ptr<http_connect>& user, int timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    int index = user->timer->index_;
    struct timeval now;
    gettimeofday(&now, NULL);
    heap_[index].deleted_ = false;
    heap_[index].expiredTime_ = (long)(now.tv_sec * 1000) + now.tv_usec / 1000 + timeout;
    UpToDown(index, size_);
}

size_t PriorityQueue::top() {
    if (size_ > 0 && !heap_[0].deleted_) {
        return heap_[0].expiredTime_;
    }
    return 0;
}

void PriorityQueue::pop() {
    --size_;
    if (size_ > 0) {
        swap(0, size_);
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
bool PriorityQueue::setDeleted(const std::shared_ptr<http_connect>& user) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!user->timer)
        return false;
    int index = user->timer->index_;
    if (index >= 0) {
        heap_[index].deleted_ = true;
        return true;
    } else {
        return false;
    }
}

void PriorityQueue::handleExpiredClient() {
    std::unique_lock<std::mutex> lock(mutex_);
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t curr = (size_t)(now.tv_sec * 1000) + now.tv_usec / 1000;
    while (size_ > 0 && (curr > top() || heap_[0].deleted_)) {
        heap_[0].user_->close_conn();
        heap_[0].user_->timer.reset();
        pop();
    }
}

int PriorityQueue::getnexttick() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (size_ == 0)
        return -1;
    size_t next = top();
    if (next == 0)
        return 0;
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t curr = (long)(now.tv_sec * 1000) + now.tv_usec / 1000;
    int ret = top() - curr;
    if (ret <= 0) {
        lock.unlock();
        handleExpiredClient();
        if (size_ == 0)
            return -1;
        ret = top() - curr;
    }
    return ret;
}
