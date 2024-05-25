## 锁机制实现多线程的同步

### **RAII**

- RAII全称是“Resource Acquisition is Initialization”，直译过来是“资源获取即初始化”.
- 在构造函数中申请分配资源，在析构函数中释放资源。因为C++的语言机制保证了，当一个对象创建的时候，自动调用构造函数，当对象超出作用域的时候会自动调用析构函数。所以，在RAII的指导下，我们应该使用类来管理资源，将资源和对象的生命周期绑定
- RAII的核心思想是将资源或者状态与对象的生命周期绑定，通过C++的语言机制，实现资源和状态的安全管理,智能指针是RAII最好的例子

### 锁的封装

对锁进行封装，实现RAII机制

## C++11锁机制

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

