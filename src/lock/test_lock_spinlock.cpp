#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 自旋锁
pthread_spinlock_t spinlock;
int shared_data = 0;

// 工作线程函数
void* worker(void* arg) {
    int thread_num = *(int*)arg;

    // 加锁
    pthread_spin_lock(&spinlock);

    // 临界区
    printf("Thread %d is in the critical section\n", thread_num);
    shared_data++;
    printf("Thread %d updated shared_data to %d\n", thread_num, shared_data);

    // 模拟长时间操作
    sleep(1);

    // 解锁
    pthread_spin_unlock(&spinlock);

    return NULL;
}

int main() {
    pthread_t threads[3];
    int thread_args[3];
    int result;

    // 初始化自旋锁
    result = pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    if (result != 0) {
        fprintf(stderr, "Failed to initialize spinlock\n");
        return EXIT_FAILURE;
    }

    // 创建线程
    for (int i = 0; i < 3; i++) {
        thread_args[i] = i + 1;
        result = pthread_create(&threads[i], NULL, worker, &thread_args[i]);
        if (result != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i + 1);
            return EXIT_FAILURE;
        }
    }

    // 等待所有线程完成
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // 销毁自旋锁
    pthread_spin_destroy(&spinlock);

    return EXIT_SUCCESS;
}