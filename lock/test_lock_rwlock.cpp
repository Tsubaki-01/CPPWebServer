#include <pthread.h>
#include <stdio.h>

pthread_rwlock_t rwlock;

void* read_function(void* arg) {
    pthread_rwlock_rdlock(&rwlock);
    // 读取共享资源
    printf("Reader thread %d is reading\n", *(int*)arg);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void* write_function(void* arg) {
    pthread_rwlock_wrlock(&rwlock);
    // 写入共享资源
    printf("Writer thread %d is writing\n", *(int*)arg);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

int main() {
    pthread_t readers[2], writer;
    int arg1 = 1, arg2 = 2, arg3 = 3;

    pthread_rwlock_init(&rwlock, NULL);

    pthread_create(&readers[0], NULL, read_function, &arg1);
    pthread_create(&writer, NULL, write_function, &arg3);
    pthread_create(&readers[1], NULL, read_function, &arg2);
    /* Reader thread 1 is reading
    Reader thread 2 is reading
    Writer thread 3 is writing
    写操作等待读操作完成后独占读写锁 */
    pthread_join(readers[0], NULL);
    pthread_join(readers[1], NULL);
    pthread_join(writer, NULL);

    pthread_rwlock_destroy(&rwlock);
    return 0;
}
