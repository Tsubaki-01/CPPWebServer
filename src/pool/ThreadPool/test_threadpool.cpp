#include <iostream>
#include <chrono>
#include <vector>
#include "threadpool.cpp"

void testFunction(int id) {
    std::cout << "Task " << id << " is starting." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟工作负载
    std::cout << "Task " << id << " is finished." << std::endl;
}

int main() {
    // 创建一个线程池对象，包含4个线程
    ThreadPool pool(4);

    // 向线程池中添加任务
    for (int i = 0; i < 8; ++i) {
        pool.addTask([i]() { testFunction(i); });
    }

    // 等待一段时间以确保所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "All tasks completed." << std::endl;
    return 0;
}
