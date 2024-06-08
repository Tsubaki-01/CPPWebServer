### 可变参数列表（variadic arguments）<cstdarg>

C++ 中的可变参数列表（variadic arguments），用于表示函数可以接受可变数量的参数。这种语法通常用于格式化字符串或处理类似的场景。

`va_list`：这是一个类型，用于声明一个变量，该变量将持有可变参数列表。

`va_start`：这个宏用于初始化 `va_list` 类型的变量，使其指向可变参数列表的第一个参数。

`va_arg`：这个宏用于访问可变参数列表中的参数。

`va_end`：这个宏用于结束对可变参数列表的访问。



```c++
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
```

```c++
#include <cstdio>
#include <cstdarg>

void formatString(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, size, format, args);
    va_end(args);
}

int main() {
    char buffer[100];
    formatString(buffer, sizeof(buffer), "Hello %s, your score is %d", "Alice", 95);
    printf("%s\n", buffer);
    return 0;
}

```

`formatString` 函数接受一个字符缓冲区、缓冲区大小、格式化字符串和可变参数。

`va_list args;` 定义一个 `va_list` 类型的变量 `args`。

`va_start(args, format);` 初始化 `args`，使其指向第一个可变参数。

`vsnprintf(buffer, size, format, args);` 将格式化的字符串输出到 `buffer` 中。

`va_end(args);` 结束对可变参数的处理。

#### **可变参数宏__VA_ARGS__**

__VA_ARGS__是一个可变参数的宏，定义时宏定义中参数列表的最后一个参数为省略号，在实际使用时会发现有时会加##，有时又不加。

```c++
1//最简单的定义
2#define my_print1(...)  printf(__VA_ARGS__)
3
4//搭配va_list的format使用
5#define my_print2(format, ...) printf(format, __VA_ARGS__)  
6#define my_print3(format, ...) printf(format, ##__VA_ARGS__)
```

__VA_ARGS__宏前面加上##的作用在于，当可变参数的个数为0时，这里printf参数列表中的的##会把前面多余的","去掉，否则会编译出错，建议使用后面这种，使得程序更加健壮。