### `struct sockaddr_in`

`struct sockaddr_in` 是一个用于存储互联网协议（IP）地址和端口信息的结构体，常用于套接字编程中。它定义在 `<netinet/in.h>` 头文件中，并且通常与 IPv4 套接字地址相关联。

以下是 `struct sockaddr_in` 的定义和各个字段的解释：

**定义**

```c++
struct sockaddr_in {
    sa_family_t    sin_family;   // 地址族（Address family）
    in_port_t      sin_port;     // 端口号（Port number）
    struct in_addr sin_addr;     // IP 地址（IP address）
    char           sin_zero[8];  // 填充字段（Padding）
};
```

**字段解释**

1. **sa_family_t sin_family**：
   - 类型：`sa_family_t`
   - 说明：指定地址族，对于 `sockaddr_in` 来说，通常设置为 `AF_INET`，表示使用 IPv4 地址。
   
2. **in_port_t sin_port**：
   - 类型：`in_port_t`
   - 说明：存储端口号，端口号必须用 `htons` 函数将主机字节序转换为网络字节序。
   
3. **struct in_addr sin_addr**：
   - 类型：`struct in_addr`
   - 说明：存储 IPv4 地址，`in_addr` 结构体包含一个名为 `s_addr` 的字段，用于存储 32 位的 IPv4 地址，地址必须用 `htonl` 函数将主机字节序转换为网络字节序。
   
4. **char sin_zero[8]**：
   - 类型：`char[8]`
   
   - 说明：填充字段，保证 `sockaddr_in` 结构与 `struct sockaddr` 的大小相同。在实际使用中一般设置为全零。

**示例**

```c++
#include <stdio.h>
#include <string.h>     // For memset
#include <sys/types.h>  // For data types used in socket
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_pton

int main() {
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // 初始化 sockaddr_in 结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // 清零

    server_addr.sin_family = AF_INET; // 地址族
    server_addr.sin_port = htons(8080); // 端口号，使用 htons 转换
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // IP 地址

    // 绑定套接字
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    // 监听连接
    if (listen(sockfd, 10) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server is listening on 127.0.0.1:8080\n");

    // 关闭套接字
    close(sockfd);

    return 0;
}
```



### `inet_pton`和`inet_ntop`

`inet_pton` 和 `inet_ntop` 是两个用于在文本和数字之间转换 IP 地址的函数。它们在网络编程中非常有用，特别是当需要将人类可读的 IP 地址转换为网络可使用的二进制格式，或将二进制格式转换回人类可读的形式时。

**`inet_pton`**

`inet_pton`（presentation to network）函数将文本形式的 IP 地址转换为二进制形式。其定义如下：

```c++
#include <arpa/inet.h>

int inet_pton(int af, const char *src, void *dst);
```

**参数**

1. **af**：地址族。可以是 `AF_INET`（IPv4）或 `AF_INET6`（IPv6）。
2. **src**：指向以 `NULL` 结尾的字符串，表示文本形式的 IP 地址。
3. **dst**：指向存储二进制形式 IP 地址的缓冲区。

**返回值**

- 成功时返回 1。
- 文本形式的 IP 地址格式无效时返回 0。
- 出现错误时返回 -1，并设置 `errno`。

**`inet_ntop`**

`inet_ntop`（network to presentation）函数将二进制形式的 IP 地址转换为文本形式。其定义如下：

```c++
#include <arpa/inet.h>

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
```

**参数**

1. **af**：地址族。可以是 `AF_INET`（IPv4）或 `AF_INET6`（IPv6）。
2. **src**：指向存储二进制形式 IP 地址的缓冲区。
3. **dst**：指向存储文本形式 IP 地址的缓冲区。
4. **size**：文本缓冲区 `dst` 的大小（以字节为单位）。

**返回值**

- 成功时返回 `dst`，即文本形式的 IP 地址。
- 出现错误时返回 `NULL`，并设置 `errno`。

### socket函数

```c++
int socket(int domain, int type, int protocol);

domain：指定套接字使用的协议族，常见的有AF_INET（IPv4）和AF_INET6（IPv6）等。
type：指定套接字的类型，常见的有SOCK_STREAM（面向连接的流套接字）和SOCK_DGRAM（无连接的数据报套接字）等。
protocol：指定协议类型，通常为0，表示根据domain和type选择默认协议。
    
返回一个新创建的套接字的文件描述符，如果出错则返回-1
```

### `setsockopt`函数

```c++
int setsockopt(int sockfd, int level, int option_name, const void *option_value, socklen_t option_len);

sockfd：要设置选项的套接字文件描述符。
level：选项所属的协议层，常见的有SOL_SOCKET表示通用套接字选项，还有其他协议层的选项。
option_name：选项的名称，可以是各种选项常量，用于指定要设置的具体选项。
    - SO_REUSEADDR：允许地址重用，即允许多个套接字绑定到同一个地址。
	- SO_KEEPALIVE：启用或禁用TCP的保活机制。
	- SO_RCVBUF：设置接收缓冲区的大小。
	- SO_SNDBUF：设置发送缓冲区的大小
    - SO_LINGER：设置linger选项
option_value：指向保存选项值的缓冲区的指针。
option_len：选项值的长度。
    
setsockopt函数的返回值为0表示设置成功，如果返回-1则表示设置失败
```

### `bind` 函数

```c++
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

sockfd：这是由socket函数返回的套接字描述符。
addr：这是一个指向sockaddr结构的指针，包含了你要绑定的地址和端口。
addrlen：这个参数指定了addr结构的长度。
    
    bind函数返回值小于0，表示绑定失败
```

### `listen`函数

```c++
int listen(int sockfd, int backlog);

sockfd：要设置为监听状态的套接字文件描述符。
backlog：连接请求队列的最大长度。
```

### `accept`函数

```C++
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

sockfd：是监听套接字的文件描述符。
addr：是一个指向struct sockaddr类型的指针，用于接收客户端的套接字地址信息。
addrlen：是一个指向socklen_t类型的指针，用于指定addr的大小，同时也会更新为实际接收到的客户端套接字地址的大小。
    
accept函数在调用时会阻塞程序，直到有客户端连接请求到达。一旦有连接请求到达，accept函数会创建一个新的套接字，并返回该新套接字的文件描述符。同时，将客户端的套接字地址信息存储在addr中，并更新addrlen为实际接收到的客户端套接字地址的大小。
```

### `send`函数

```C++
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

sockfd：是要发送数据的套接字的文件描述符。
buf：是一个指向要发送的数据的指针。
len：是要发送的数据的长度。
flags：是一个标志参数，用于指定发送数据时的特殊选项。
    
    失败返回负值
    
send函数是一个阻塞函数，即在发送数据时可能会阻塞程序，直到数据完全发送或者发生错误。如果需要非阻塞发送，可以通过设置套接字为非阻塞模式或使用select等函数来进行管理。
```

