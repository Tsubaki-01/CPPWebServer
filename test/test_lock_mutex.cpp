#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock;

void* thread_function(void* arg)
{
    pthread_mutex_lock(&lock);
    // 访问共享资源
    printf("Thread %d is in the critical section\n", *(int*)arg);
    pthread_mutex_unlock(&lock);
    return NULL;
}


int main() {
    pthread_t thread1, thread2;
    int arg1 = 1, arg2 = 2;

    pthread_mutex_init(&lock, NULL);

    pthread_create(&thread1, NULL, thread_function, &arg1);
    pthread_create(&thread2, NULL, thread_function, &arg2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}