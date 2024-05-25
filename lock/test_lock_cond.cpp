#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int ready = 0;

void* waiter_function(void* arg) {
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("Thread %d received signal\n", *(int*)arg);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* signaler_function(void* arg) {
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    printf("Thread %d post signal\n", *(int*)arg);
    pthread_mutex_unlock(&mutex);
    /* 在 pthread_cond_wait(&cond, &mutex); 被唤醒之后，线程会等待获取互斥锁 mutex，只有在成功获取锁之后才会返回并继续执行。这意味着 waiter_thread 会等待 signaler_thread 释放 mutex 之后，才能重新获取锁并继续执行。 */
    return NULL;
}

int main() {
    pthread_t waiter, signaler;
    int arg1 = 1, arg2 = 2;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&waiter, NULL, waiter_function, &arg1);
    pthread_create(&signaler, NULL, signaler_function, &arg2);

    pthread_join(waiter, NULL);
    pthread_join(signaler, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}