#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

sem_t semaphore;

void* thread_function(void* arg) {
    sem_wait(&semaphore);
    // 访问共享资源
    printf("Thread %d is in the critical section\n", *(int*)arg);
    sem_post(&semaphore);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int args[3] = { 1, 2, 3 };

    sem_init(&semaphore, 0, 2);

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_function, &args[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaphore);
    return 0;
}
