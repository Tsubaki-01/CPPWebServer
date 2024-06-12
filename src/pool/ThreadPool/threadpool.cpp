#include "threadpool.h"

ThreadPool::ThreadPool(size_t threadNum) :isClosed_(false)
{
    assert(threadNum > 0);
    // 创建线程池
    for (size_t i = 0;i < threadNum;i++)
    {
        threadPool.emplace_back(std::thread(&ThreadPool::worker, this));
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
