// encode UTF-8

// @Author        : Aged_cat
// @Date          : 2021-05-04

#ifndef THREADPOOLH
#define THREADPOOLH

#include<thread>
#include<condition_variable>
#include<mutex>
#include<vector>
#include<queue>
#include<future>
#include <functional>
#include <sys/time.h>
#include <signal.h>
#include <string.h>

class ThreadPool{
private:
    bool m_stop;
    std::vector<std::thread>m_thread;
    std::queue<std::function<void()>>tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    // int leftthread;

public:
    explicit ThreadPool(size_t threadNumber):m_stop(false) {
        struct sigaction sa;
        memset(&sa, '\0', sizeof(sa));
        sa.sa_handler = SIG_IGN;
        sigfillset(&sa.sa_mask);
        sigaction(SIGPIPE, &sa, NULL);
        for(size_t i=0;i<threadNumber;++i)
        {
            m_thread.emplace_back(
                [this](){
                    for(;;)
                    {
                        std::function<void()>task;
                        {
                            std::unique_lock<std::mutex> lock(m_mutex);
                            m_cv.wait(lock,[this](){ return m_stop||!tasks.empty();});
                            if(m_stop&&tasks.empty()) return;
                            // leftthread--;
                            // LOG << "thread left : " << leftthread << '\n';
                            task=std::move(tasks.front());
                            tasks.pop();
                        }
                        // struct timeval t1, t2;
                        // gettimeofday(&t1, NULL);
                        task();
                        // gettimeofday(&t2, NULL);
                        // LOG << "time use : " << (long)(t2.tv_sec - t1.tv_sec) * 1000 + (int)(t2.tv_usec - t1.tv_usec)/1000 << "\n";
                        // leftthread++;
                        // LOG << "thread left : " << leftthread << '\n';
                    }
                }
            );
        }
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;

    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;

    ~ThreadPool(){
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop=true;
        }
        m_cv.notify_all();
        for(auto& threads:m_thread)
        {
            threads.join();
        }
    }

    template<typename F,typename... Args>
    auto submit(F&& f,Args&&... args)->std::future<decltype(f(args...))>{
        auto taskPtr=std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
        );
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(m_stop) throw std::runtime_error("submit on stopped ThreadPool");
            tasks.emplace([taskPtr](){ (*taskPtr)(); });
        }
        m_cv.notify_one();
        return taskPtr->get_future();

    }
};

#endif //THREADPOOL_H
