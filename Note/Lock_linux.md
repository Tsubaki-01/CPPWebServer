## linux中的锁机制

在Linux中，锁机制用于在并发环境中保护共享资源，防止数据竞争和不一致性。Linux提供了多种锁机制，适用于不同的场景和需求。以下是几种常见的锁机制：

### 1. 互斥锁 (Mutex)

互斥锁是最常用的锁机制，用于保护共享资源，确保同时只有一个线程能够访问资源。

互斥锁属于sleep-waiting类型的锁，例如在一个双核的机器上有两个线程（线程A和线程B）,它们分别运行在Core0和Core1上。假设线程A想要通过pthread_mutex_lock操作去得到一个临界区的锁，而此时这个锁正被线程B所持有，那么线程A就会被阻塞，Core0会在此时进行上下文切换(Context Switch)将线程A置于等待队列中，此时Core0就可以运行其它的任务而不必进行忙等待。

**适用场景：**

因互斥锁会引起线程的切换，效率较低。使用互斥锁会引起线程阻塞等待，不会一直占用这cpu，因此当锁的内容较多，切换不频繁时，建议使用互斥锁

互斥锁适用于那些可能会阻塞很长时间的场景。

1、 临界区有IO操作

2 、临界区代码复杂或者循环量大

3 、临界区竞争非常激烈

4、 单核处理器

#### `pthread_mutex` 函数族

互斥锁（Mutex）用于保护共享资源，确保同一时刻只有一个线程能够访问资源。

- **`int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);`**
  初始化互斥锁。
- **`int pthread_mutex_destroy(pthread_mutex_t *mutex);`**
  销毁互斥锁。
- **`int pthread_mutex_lock(pthread_mutex_t *mutex);`**
  加锁，如果锁已经被其他线程持有，则阻塞直到锁可用。
- **`int pthread_mutex_trylock(pthread_mutex_t *mutex);`**
  尝试加锁，如果锁不可用则立即返回错误。
- **`int pthread_mutex_unlock(pthread_mutex_t *mutex);`**
  解锁。

#### 使用互斥锁的例子

```c++
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock;

void *thread_function(void *arg) {
    pthread_mutex_lock(&lock);
    // 访问共享资源
    printf("Thread %d is in the critical section\n", *(int *)arg);
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
```

### 2. 自旋锁 (Spinlock)

自旋锁是一个轻量级的锁，适用于锁定时间非常短的临界区。自旋锁在获取锁时不会让线程休眠，而是会忙等待。

Spin lock（自旋锁）属于busy-waiting类型的锁，如果线程A是使用pthread_spin_lock操作去请求锁，那么线程A就会一直在Core0上进行忙等待并不停的进行锁请求，直到得到这个锁为止。自旋锁不会引起调用者睡眠，如果自旋锁已经被别的执行单元保持，调用者就一直循环在那里看是否该自旋锁的保持者已经释放了锁。

**优点**：

- **低延迟**：自旋锁不会引起调用者睡眠不会引起线程的上下文切换，因此在锁等待时间非常短的情况下，能够减少开销，提高性能。
- **简单实现**：自旋锁的实现相对简单，适合在简单的多处理器系统中使用。

**缺点**：

- **忙等待**：自旋锁在等待锁时会消耗 CPU 资源，如果锁等待时间较长，会导致性能下降。

- **不适合单处理器系统**：在单处理器系统中，自旋锁可能会导致死锁，因为持有锁的线程无法释放锁。

- **不适用于长时间持有的锁**：如果需要长时间持有锁，自旋锁会导致其他线程浪费大量 CPU 时间。

#### 自旋锁的使用场景

- **短时间锁持有**：自旋锁适用于锁持有时间非常短的场景，例如保护短时间内访问的变量或状态标志。
- **高并发环境**：在多处理器系统中，自旋锁能够有效地减少线程间的上下文切换开销。
- **临界区代码简单**：自旋锁适用于临界区代码较简单、不涉及复杂操作的情况。

#### `pthread_spinlock` 函数族

- **`int pthread_spin_init(pthread_spinlock_t *lock, int pshared);`**
  初始化自旋锁。
- **`int pthread_spin_destroy(pthread_spinlock_t *lock);`**
  销毁自旋锁。
- **`int pthread_spin_lock(pthread_spinlock_t *lock);`**
  加锁，自旋等待直到锁可用。
- **`int pthread_spin_trylock(pthread_spinlock_t *lock);`**
  尝试加锁，如果锁不可用则立即返回错误。
- **`int pthread_spin_unlock(pthread_spinlock_t *lock);`**
  解锁。

#### 使用自旋锁的例子

```c++
#include <pthread.h>
#include <stdio.h>

pthread_spinlock_t spinlock;

void *thread_function(void *arg) {
    pthread_spin_lock(&spinlock);
    // 访问共享资源
    printf("Thread %d is in the critical section\n", *(int *)arg);
    pthread_spin_unlock(&spinlock);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int arg1 = 1, arg2 = 2;

    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);

    pthread_create(&thread1, NULL, thread_function, &arg1);
    pthread_create(&thread2, NULL, thread_function, &arg2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_spin_destroy(&spinlock);
    return 0;
}
```

### 3. 读写锁 (Read-Write Lock)

读写锁允许多个线程同时读取共享资源，但在写操作时必须独占锁。读写锁适用于读多写少的场景。

#### `pthread_rwlock` 函数族

- **`int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);`**
  初始化读写锁。
- **`int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);`**
  销毁读写锁。
- **`int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);`**
  获取读锁，如果写锁已被持有则阻塞。
- **`int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);`**
  尝试获取读锁，如果锁不可用则立即返回错误。
- **`int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);`**
  获取写锁，如果锁已被其他线程持有则阻塞。
- **`int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);`**
  尝试获取写锁，如果锁不可用则立即返回错误。
- **`int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);`**
  解锁。

#### 使用读写锁的例子

```c++
#include <pthread.h>
#include <stdio.h>

pthread_rwlock_t rwlock;

void *read_function(void *arg) {
    pthread_rwlock_rdlock(&rwlock);
    // 读取共享资源
    printf("Reader thread %d is reading\n", *(int *)arg);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *write_function(void *arg) {
    pthread_rwlock_wrlock(&rwlock);
    // 写入共享资源
    printf("Writer thread %d is writing\n", *(int *)arg);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

int main() {
    pthread_t readers[2], writer;
    int arg1 = 1, arg2 = 2, arg3 = 3;

    pthread_rwlock_init(&rwlock, NULL);

    pthread_create(&readers[0], NULL, read_function, &arg1);
    pthread_create(&readers[1], NULL, read_function, &arg2);
    pthread_create(&writer, NULL, write_function, &arg3);

    pthread_join(readers[0], NULL);
    pthread_join(readers[1], NULL);
    pthread_join(writer, NULL);

    pthread_rwlock_destroy(&rwlock);
    return 0;
}
```

### 4. 信号量 (Semaphore)

信号量是一种广义的锁机制，可以控制多个线程对共享资源的访问。信号量可以是计数信号量，用于限制对资源的同时访问线程数。

#### `sem` 函数族(PV操作)

- **`int sem_init(sem_t *sem, int pshared, unsigned int value);`**
  初始化信号量。
  
  `pshared`参数控制着信号量的类型。如果 `pshared`的值是0，就表示它是当前进程的局部信号量；否则，其它进程就能够共享这个信号量。只对不让进程共享的信号量感兴趣。Linux线程一般不支持进程间共享信号量，`pshared`传递一个非零将会使函数返回ENOSYS错误。
  
- **`int sem_destroy(sem_t *sem);`**
  销毁信号量。
  
- **`int sem_wait(sem_t *sem);`**
  等待信号量，递减信号量值，如果值为 0 则阻塞。
  
- **`int sem_trywait(sem_t *sem);`**
  尝试等待信号量，如果信号量值为 0 则立即返回错误。
  
- **`int sem_post(sem_t *sem);`**
  增加信号量值，如果有阻塞的线程则唤醒。
  
- **`int sem_getvalue(sem_t *sem, int *sval);`**
  获取信号量的当前值。

#### 使用信号量的例子

```c++
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

sem_t semaphore;

void *thread_function(void *arg) {
    sem_wait(&semaphore);
    // 访问共享资源
    printf("Thread %d is in the critical section\n", *(int *)arg);
    sem_post(&semaphore);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int args[3] = {1, 2, 3};

    sem_init(&semaphore, 0, 1);

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_function, &args[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaphore);
    return 0;
}
```

### 5. 条件变量 (Condition Variable)

条件变量用于线程间的通信，使一个线程可以阻塞自己直到另一个线程发出某个条件信号。

#### `pthread_cond` 函数族

- **`int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);`**
  初始化条件变量。
- **`int pthread_cond_destroy(pthread_cond_t *cond);`**
  销毁条件变量。
- **`int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);`**
  等待条件变量，等待期间释放互斥锁，条件满足后重新获取锁。
- **`int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);`**
  等待条件变量，直到指定的绝对时间 `abstime` 超时。期间释放互斥锁，条件满足或超时后重新获取锁。
- **`int pthread_cond_signal(pthread_cond_t *cond);`**
  唤醒至少一个等待该条件变量的线程。
- **`int pthread_cond_broadcast(pthread_cond_t *cond);`**
  唤醒所有等待该条件变量的线程。

#### 使用条件变量的例子

```c++
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int ready = 0;

void *waiter_function(void *arg) {
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("Thread %d received signal\n", *(int *)arg);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *signaler_function(void *arg) {
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
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
```

