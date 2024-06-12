### `epoll_create`和`epoll_create1`的区别

`epoll_create`和`epoll_create1`都是用于创建epoll实例的函数，但它们有以下区别：

1. 参数不同：
   - `epoll_create`函数的参数是`size`，表示期望监听的文件描述符数量，但该参数在实际使用中并没有什么实际意义。
   - `epoll_create1`函数的参数是`flags`，表示创建epoll实例时的标志。可以使用`EPOLL_CLOEXEC`标志来设置在执行exec时关闭epoll实例。
2. 返回值不同：
   - `epoll_create`函数返回一个epoll实例的文件描述符，如果出错则返回-1。
   - `epoll_create1`函数返回一个epoll实例的文件描述符，如果出错则返回-1。
3. 错误处理不同：
   - `epoll_create`函数在出错时会设置errno来指示错误的原因。
   - `epoll_create1`函数在出错时会直接返回-1。

总体而言，`epoll_create1`是`epoll_create`的一个增强版本，它提供了更简洁的参数和错误处理方式。在使用时，推荐使用`epoll_create1`函数来创建epoll实例。

### `struct epoll_event`

  结构体`epoll_event`被用于注册所感兴趣的事件和回传所发生待处理的事件，定义如下：

```c++
typedef union epoll_data {
    void *ptr;
     int fd;
     __uint32_t u32;
     __uint64_t u64;
   } epoll_data_t;//保存触发事件的某个文件描述符相关的数据

   struct epoll_event {
     __uint32_t events;   /* epoll event */
     epoll_data_t data;   /* User data variable */
   };
```

  其中events表示感兴趣的事件和被触发的事件，可能的取值为：
  EPOLLIN：表示对应的文件描述符可以读；
  EPOLLOUT：表示对应的文件描述符可以写；
  EPOLLPRI：表示对应的文件描述符有紧急的数可读；

  EPOLLERR：表示对应的文件描述符发生错误；
  EPOLLHUP：表示套接字被挂起，当套接字被挂起时（例如对端关闭连接），这个事件将被触发。
  EPOLLRDHUP：read hang up监测套接字的对端关闭连接
  EPOLLONESHOT：表示一次性触发模式，当使用一次性触发模式时，epoll 在触发事件后会自动将套接字从监听集合中删除，需要重新添加监听才能继续监视。
  EPOLLET：  ET的epoll工作模式；



### `epoll_ctl`函数

```C++
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
功能：用于控制某个文件描述符上的事件，可以注册事件，修改事件，删除事件。
   
@epfd：由epoll_create生成的epoll专用的文件描述符；
@op：要进行的操作，EPOLL_CTL_ADD注册、EPOLL_CTL_MOD修改、EPOLL_CTL_DEL删除；
@fd：关联的文件描述符；  
@event：指向epoll_event的指针；
  
成功：0；失败：-1
```

### `epoll_wait`函数

```C++
int epoll_wait(int epfd,struct epoll_event * events,int maxevents,int timeout)

  功能：该函数用于轮询I/O事件的发生；

  @epfd：由epoll_create生成的epoll专用的文件描述符；

  @epoll_event：用于回传代处理事件的数组；

  @maxevents：每次能处理的事件数；

  @timeout：等待I/O事件发生的超时值；

  成功：返回发生的事件数；失败：-1
```

**应用举例：**

```C++
#define SERV_PORT 4466  //服务端口号
const char* LOCAL_ADDR = "127.0.0.1";//绑定服务地址

bool setnonblocking(int sock)//设置socket为非阻塞方式
{
    int opts;

    opts = fcntl(sock, );
    if (opts < 0)
    {
        perror("fcntl(sock,GETFL)");
        return false;
    }
    opts = opts | ;
    if (fcntl(sock, , opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return false;
    }
    return true;
}

int main()
{
    int i, maxi, listenfd, new_fd, sockfd, epfd, nfds;
    ssize_t n;
    char line[MAXLINE];
    socklen_t clilen;

    struct epoll_event ev, events[20];//ev用于注册事件,数组用于回传要处理的事件

    struct sockaddr_in clientaddr, serveraddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);//生成socket文件描述符

    setnonblocking(listenfd);//把socket设置为非阻塞方式

    epfd = epoll_create(256);//生成用于处理accept的epoll专用的文件描述符

    ev.data.fd = listenfd;//设置与要处理的事件相关的文件描述符

    ev.events = EPOLLIN | EPOLLET;//设置要处理的事件类型

    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);//注册epoll事件

    //设置服务器端地址信息
    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;

    char* local_addr = LOCAL_ADDR;

    inet_aton(local_addr, &(serveraddr.sin_addr));

    serveraddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (sockaddr*)&serveraddr, sizeof(serveraddr));//绑定socket连接
    listen(listenfd, LISTENQ);//监听

    maxi = 0;
    for (; ; )
    {
        /* epoll_wait：等待epoll事件的发生，并将发生的sokct fd和事件类型放入到events数组中；
        \* nfds：为发生的事件的个数。
        \* 注：
        */
        nfds = epoll_wait(epfd, events, 20, 500);

        //处理所发生的所有事件
        for (i = 0;i < nfds;++i)
        {
            if (events[i].data.fd == listenfd)//事件发生在listenfd上
            {
                /* 获取发生事件端口信息，存于clientaddr中；
                *new_fd：返回的新的socket描述符，用它来对该事件进行recv/send操作*/
                new_fd = accept(listenfd, (struct sockaddr*)&clientaddr, &clilen);
                if (connfd < 0)
                {
                    perror("connfd<0");
                    exit(1);
                }
                setnonblocking(connfd);

                char* str = inet_ntoa(clientaddr.sin_addr);

                ev.data.fd = connfd;//设置用于读操作的文件描述符

                ev.events = EPOLLIN | EPOLLET;//设置用于注测的读操作事件

                epoll_ctl(epfd, , , &ev);//注册ev
            }
            else if (events[i].events & EPOLLIN)
            {
                if ((sockfd = events[i].data.fd) < 0)
                    continue;

                if ((n = read(sockfd, line, MAXLINE)) < 0)
                {
                    if (errno == ECONNRESET)
                    {
                        close(sockfd);
                        events[i].data.fd = -1;
                    }
                    else
                        std::cout << "readline error" << std::endl;
                }
                else if (n == 0)
                {
                    close(sockfd);
                    events[i].data.fd = -1;
                }
                ev.data.fd = sockfd;//设置用于写操作的文件描述符

                ev.events = EPOLLOUT | EPOLLET;//设置用于注测的写操作事件

                epoll_ctl(epfd, , sockfd, &ev);//修改sockfd上要处理的事件为EPOLLOUT

```

