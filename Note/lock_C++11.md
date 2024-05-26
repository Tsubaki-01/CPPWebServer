## 锁机制

C++11 引入了新的标准库组件和特性，使得多线程编程和锁机制的实现更加简单和安全。以下是 C++11 中用于实现锁机制的几个关键组件和它们的用法：

### 1. `std::mutex`

`std::mutex` 是一种用于实现互斥锁的基本机制，可以用来保护共享资源，确保同时只有一个线程可以访问该资源。

```C++
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void print_thread_id(int id) {
    mtx.lock();
    std::cout << "Thread " << id << std::endl;
    mtx.unlock();
}

int main() {
    std::thread threads[10];
    for (int i = 0; i < 10; ++i) {
        threads[i] = std::thread(print_thread_id, i);
    }

    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
```

### 2. `std::lock_guard`

`std::lock_guard` 是一个轻量级的RAII（Resource Acquisition Is Initialization）风格的锁管理器，用于自动获取和释放锁。

```C++
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void print_thread_id(int id) {
    std::lock_guard<std::mutex> lock(mtx);  // 自动获取和释放锁
    std::cout << "Thread " << id << std::endl;
}

int main() {
    std::thread threads[10];
    for (int i = 0; i < 10; ++i) {
        threads[i] = std::thread(print_thread_id, i);
    }

    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
```

### 3. `std::unique_lock`

`std::unique_lock` 提供了更灵活的锁管理，可以显式地锁定和解锁，并且支持延迟锁定和所有权转移。

```C++
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void print_thread_id(int id) {
    std::unique_lock<std::mutex> lock(mtx);  // 自动获取和释放锁
    std::cout << "Thread " << id << std::endl;
    // lock.unlock();  // 显式解锁
    // lock.lock();    // 显式加锁
}

int main() {
    std::thread threads[10];
    for (int i = 0; i < 10; ++i) {
        threads[i] = std::thread(print_thread_id, i);
    }

    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
```

### 4. `std::shared_mutex` 和 `std::shared_lock`

C++17 引入了 `std::shared_mutex` 和 `std::shared_lock`，允许多线程共享读锁和独占写锁，适用于读多写少的场景。

```C++
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>

std::shared_mutex sh_mtx;
std::vector<int> data;

void reader(int id) {
    std::shared_lock<std::shared_mutex> lock(sh_mtx);  // 共享锁
    std::cout << "Reader " << id << " read data: ";
    for (int val : data) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

void writer(int id, int value) {
    std::unique_lock<std::shared_mutex> lock(sh_mtx);  // 独占锁
    std::cout << "Writer " << id << " writing data: " << value << std::endl;
    data.push_back(value);
}

int main() {
    std::thread readers[5], writers[5];

    for (int i = 0; i < 5; ++i) {
        readers[i] = std::thread(reader, i);
        writers[i] = std::thread(writer, i, i * 10);
    }

    for (int i = 0; i < 5; ++i) {
        readers[i].join();
        writers[i].join();
    }

    return 0;
}
```

### 5. `std::condition_variable`

`std::condition_variable` 用于线程之间的等待和通知机制，常用于实现生产者-消费者模型。

```C++
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> data_queue;

void producer(int id) {
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::unique_lock<std::mutex> lock(mtx);
        data_queue.push(id * 10 + i);
        std::cout << "Producer " << id << " produced " << id * 10 + i << std::endl;
        cv.notify_one();
    }
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !data_queue.empty(); });
        int value = data_queue.front();
        data_queue.pop();
        std::cout << "Consumer " << id << " consumed " << value << std::endl;
        if (value == 40) break;  // Example condition to stop consumer
    }
}

int main() {
    std::thread prod1(producer, 1);
    std::thread prod2(producer, 2);
    std::thread cons(consumer, 1);

    prod1.join();
    prod2.join();
    cons.join();

    return 0;
}
```

### 6.  信号量

C++11 标准库中没有直接包含信号量（semaphore）的实现，但在 C++20 中引入了 `std::counting_semaphore` 和 `std::binary_semaphore`。在 C++11 中，如果需要使用信号量，可以通过第三方库（如 Boost）或使用 POSIX 信号量（在类 Unix 系统上）。

#### 使用 `std::counting_semaphore`（C++20）

如果可以使用 C++20 标准，标准库中提供了 `std::counting_semaphore` 和 `std::binary_semaphore`。

```C++
#include <iostream>
#include <thread>
#include <semaphore>

std::counting_semaphore<3> sem(3);  // 初始化信号量，允许最多 3 个线程同时工作

void worker(int id) {
    sem.acquire();  // 等待信号量
    std::cout << "Worker " << id << " is working" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 模拟工作
    sem.release();  // 释放信号量
}

int main() {
    std::thread workers[5];
    for (int i = 0; i < 5; ++i) {
        workers[i] = std::thread(worker, i);
    }

    for (auto& worker : workers) {
        worker.join();
    }

    return 0;
}
```

#### 自定义信号量类（C++11）

在没有信号量支持的情况下，可以使用互斥锁和条件变量来自行实现一个简单的信号量。

```C++
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore(int count = 0) : count(count) {}

    void signal() {
        std::unique_lock<std::mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        --count;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};

Semaphore sem(3);  // 初始化信号量，允许最多 3 个线程同时工作

void worker(int id) {
    sem.wait();  // 等待信号量
    std::cout << "Worker " << id << " is working" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 模拟工作
    sem.signal();  // 释放信号量
}

int main() {
    std::thread workers[5];
    for (int i = 0; i < 5; ++i) {
        workers[i] = std::thread(worker, i);
    }

    for (auto& worker : workers) {
        worker.join();
    }

    return 0;
}
```

