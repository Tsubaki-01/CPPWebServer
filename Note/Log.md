## 多生产者单消费者模式



## 由于⽇志输⼊的类型繁多，为了后端写⼊⽅便（也为了缓冲 区的格式统⼀），需要将输⼊的数据转换为 char 字符类型再进⾏写⼊。这样就很有意思了， int 类型的转成字 符数组很容易实现（leetcode 简单题的⽔平），但如果是浮点数或者指针呢？

**综合示例**

下面是一个综合示例，展示如何处理所有这些类型并将它们转换为 `char` 数组。

```c++
#include <iostream>
#include <string>
#include <cstdio> // for sprintf

void intToCharArray(int value, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%d", value);
}

void floatToCharArray(double value, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%.6f", value);
}

void pointerToCharArray(void* ptr, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%p", ptr);
}

int main() {
    int intValue = 42;
    double doubleValue = 3.141592653589793;
    int* ptrValue = &intValue;

    char buffer[50];

    // 转换整数
    intToCharArray(intValue, buffer, sizeof(buffer));
    std::cout << "Integer: " << buffer << std::endl;

    // 转换浮点数
    floatToCharArray(doubleValue, buffer, sizeof(buffer));
    std::cout << "Double: " << buffer << std::endl;

    // 转换指针
    pointerToCharArray(ptrValue, buffer, sizeof(buffer));
    std::cout << "Pointer: " << buffer << std::endl;

    return 0;
}
```

**总结**

- 使用 `snprintf` 可以安全地将整数、浮点数和指针转换为 `char` 数组。
- 通过指定格式，可以控制输出的精度和格式。
- 将这些函数封装起来，可以方便地处理不同类型的数据并将其转换为统一的 `char` 数组格式，适用于日志记录和数据序列化。

```C++
int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *buf = str;
    size_t remaining = size;

    while (*format) {
        if (*format == '%' && *(format + 1)) {
            format++;
            // Handle different format specifiers, e.g., %d, %s, %f
            switch (*format) {
                case 'd': {
                    int value = va_arg(args, int);
                    int len = snprintf_int(buf, remaining, value);
                    buf += len;
                    remaining -= len;
                    break;
                }
                case 's': {
                    const char *value = va_arg(args, const char*);
                    int len = snprintf_str(buf, remaining, value);
                    buf += len;
                    remaining -= len;
                    break;
                }
                // Handle other format specifiers...
                default:
                    // Handle unknown specifier
                    break;
            }
        } else {
            if (remaining > 1) {
                *buf++ = *format;
                remaining--;
            }
        }
        format++;
    }

    if (remaining > 0) {
        *buf = '\0';
    } else if (size > 0) {
        *(buf - 1) = '\0';
    }

    va_end(args);
    // Return the number of characters that would have been written
    return buf - str;
}

int snprintf_int(char *buf, size_t size, int value) {
    // Implement integer to string conversion, writing to buf
    // and return the number of characters written
}

int snprintf_str(char *buf, size_t size, const char *value) {
    // Copy string to buf and return the number of characters written
}
```

### **为什么判断线程执行的条件用while而不是if？**

一般来说，在多线程资源竞争的时候，在一个使用资源的线程里面（消费者）判断资源是否可用，不可用，便调用`pthread_cond_wait`，在另一个线程里面（生产者）如果判断资源可用的话，则调用`pthread_cond_signal`发送一个资源可用信号。

在wait成功之后，资源就一定可以被使用么？答案是否定的，如果同时有两个或者两个以上的线程正在等待此资源，wait返回后，资源可能已经被使用了。

再具体点，有可能多个线程都在等待这个资源可用的信号，信号发出后只有一个资源可用，但是有A，B两个线程都在等待，B比较速度快，获得互斥锁，然后加锁，消耗资源，然后解锁，之后A获得互斥锁，但A回去发现资源已经被使用了，它便有两个选择，一个是去访问不存在的资源，另一个就是继续等待，那么继续等待下去的条件就是使用while，要不然使用if的话`pthread_cond_wait`返回后，就会顺序执行下去。

所以，在这种情况下，应该使用while而不是if:

```c++
1while(resource == FALSE)
2    pthread_cond_wait(&cond, &mutex);
```

如果只有一个消费者，那么使用if是可以的。
