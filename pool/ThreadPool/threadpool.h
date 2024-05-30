#ifndef THREADPOOL_H

#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <exception>
#include <cassert>
#include <thread>


class ThreadPool
{
public:
    explicit ThreadPool(size_t threadNum = 8); // 防止隐式转换（与复制列表初始化）
    ThreadPool(ThreadPool&&) = default;
    ~ThreadPool();

    void worker();

    // 添加任务到任务队列
    template<class T>
    void addTask(T&& task);


private:
    std::mutex mtx_; // 管理tasks的互斥锁
    std::condition_variable cv_;
    bool isClosed_; // 关闭线程  // 多线程情况下，初始化线程池时后面的线程创建时，线程池可能已经被释放删除，需要检查

    std::vector<std::thread> threadPool;
    std::queue < std::function <void()>> tasks_; // 任务队列

};


ThreadPool::ThreadPool(size_t threadNum) :isClosed_(false)
{
    assert(threadNum > 0);
    // 创建线程池
    for (size_t i = 0;i < threadNum;i++)
    {
        threadPool.emplace_back(std::thread(&worker, this));
        // threadPool[i].detach();
        /* 调用detach表示thread对象和其表示的线程完全分离，即销毁这个对象了，就不能用threadPool[i]表示？不能用容器管理了 */
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        isClosed_ = true;
    }
    cv_.notify_all(); // 退出线程
    for (std::thread& thread : threadPool)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void ThreadPool::worker()
{

    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this] { return isClosed_ || !tasks_.empty(); }); // 除关闭线程池外，只有任务队列为空时才等待
            if (isClosed_ && tasks_.empty())
            {
                return; // 退出线程
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task(); // 执行任务
    }
}

template<class T>
void ThreadPool::addTask(T&& task)
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        tasks_.emplace(std::forward<T>(task));
    }
    cv_.notify_one();
}

#endif // ~THREADPOOL_H