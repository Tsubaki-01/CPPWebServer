## pthread

### `int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);`

该函数用于创建一个新线程，并让新线程执行指定的函数。

- **参数**：
  - `pthread_t *thread`：指向线程标识符的指针，创建成功后，该标识符用于标识新线程。
  - `pthread_attr_t *attr`：线程属性对象指针，若为 NULL，则使用默认属性。
  - `void *(*start_routine)(void *)`：新线程将执行的函数，该函数必须接受一个 `void*` 类型的参数，并返回一个 `void*` 类型的结果。
  - `void *arg`：传递给 `start_routine` 的参数。
- **返回值**：成功时返回 0，失败时返回错误码。

### `int pthread_join(pthread_t tid, void **thread_return);`

该函数用于等待指定的线程终止，并获取线程的返回值。

- **参数**：
  - `pthread_t tid`：要等待的线程的标识符。
  - `void **thread_return`：指向存储线程返回值的指针，如果不需要返回值，可以传递 NULL。
- **返回值**：成功时返回 0，失败时返回错误码。

### `int pthread_detach(pthread_t tid);`

该函数将指定的线程设置为分离状态，分离状态的线程终止时，资源会自动释放，不需要 `pthread_join` 来清理。

- **参数**：
  - `pthread_t tid`：要分离的线程的标识符。
- **返回值**：成功时返回 0，失败时返回错误码。

### `void pthread_exit(void *retval);`

该函数用于终止调用它的线程，并返回一个值给 `pthread_join`。

- **参数**：
  - `void *retval`：线程的返回值，将传递给 `pthread_join` 中的 `thread_return`。
- **返回值**：该函数没有返回，因为调用它的线程将被终止。

### `int pthread_setcancelstate(int state, int *oldstate);`

该函数用于设置线程的取消状态，并可选择性地保存旧的取消状态。

- **参数**：
  - `int state`：新的取消状态，可以是 `PTHREAD_CANCEL_ENABLE` 或 `PTHREAD_CANCEL_DISABLE`。
  - `int *oldstate`：指向存储旧的取消状态的指针，如果不需要保存旧状态，可以传递 NULL。
- **返回值**：成功时返回 0，失败时返回错误码。

### `int pthread_cancel(pthread_t tid);`

该函数用于向指定的线程发送取消请求。

- **参数**：
  - `pthread_t tid`：要取消的线程的标识符。
- **返回值**：成功时返回 0，失败时返回错误码。

### 线程属性

线程属性通过 `pthread_attr_t` 类型的对象来管理。这些属性包括线程的堆栈大小、分离状态、调度策略和优先级等。

#### 常用的线程属性函数

- **初始化和销毁属性对象**：

```C++
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
```

- **设置和获取堆栈大小**：

```C++
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
```

- **设置和获取分离状态**：

```C++
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
```

### 分离状态属性

分离状态属性决定了线程的生命周期管理方式。有两种状态：

1. **可连接（Joinable）**：默认状态。主线程可以使用 `pthread_join` 等待子线程结束，并获取其退出状态。
2. **分离（Detached）**：线程一旦结束，其资源会自动释放，不能使用 `pthread_join` 等待该线程。

#### 分离状态的常量

- `PTHREAD_CREATE_JOINABLE`：可连接状态。
- `PTHREAD_CREATE_DETACHED`：分离状态。

### 使用示例

下面是一个示例，展示如何使用线程属性和分离状态属性创建线程。

```c++
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread_function(void *arg) {
    printf("Thread %d is running\n", *(int *)arg);
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_attr_t attr;
    int thread_arg = 1;
    int result;

    // 初始化线程属性对象
    result = pthread_attr_init(&attr);
    if (result != 0) {
        fprintf(stderr, "Failed to initialize thread attributes\n");
        return EXIT_FAILURE;
    }

    // 设置线程为分离状态
    result = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (result != 0) {
        fprintf(stderr, "Failed to set detach state\n");
        pthread_attr_destroy(&attr);
        return EXIT_FAILURE;
    }

    // 创建线程
    result = pthread_create(&thread, &attr, thread_function, &thread_arg);
    if (result != 0) {
        fprintf(stderr, "Failed to create thread\n");
        pthread_attr_destroy(&attr);
        return EXIT_FAILURE;
    }

    // 销毁线程属性对象
    result = pthread_attr_destroy(&attr);
    if (result != 0) {
        fprintf(stderr, "Failed to destroy thread attributes\n");
        return EXIT_FAILURE;
    }

    // 主线程不需要等待分离状态的线程
    printf("Main thread is continuing\n");

    // 让主线程睡眠以确保分离线程有时间执行
    // 在实际应用中，主线程的生命周期应该由其他逻辑控制
    sleep(1);

    return EXIT_SUCCESS;
}
```

