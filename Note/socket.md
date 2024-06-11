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