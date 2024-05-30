**维护一个线程数组，采用.join来回收资源。**

不使用.detach()的原因是调用detach即表示thread对象和其表示的线程完全分离，即销毁这个对象了，就不能用`threadPool[i]`表示，也就是不能用容器管理了

**`worker`**

```c++
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
```

1. 最开始初始化时，全部线程停止在等待条件变量的环节

2. `addTask()`后唤醒线程

3. 关闭线程池时`isClosed_`被置为true，唤醒所有线程。此时

   - 若任务数量小于线程数量，会有部分线程处于等待状态，重新开始运行，退出线程。（此时任务队列一定为空）

   - 若任务数量大于等于线程数量，所有线程都在进行工作，不会有等待的线程。

     运行一段时间后变为任务数量小于线程数量的情况。