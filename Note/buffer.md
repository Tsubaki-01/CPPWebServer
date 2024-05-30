<https://blog.csdn.net/T_Solotov/article/details/124044175>
该文章中的buffer类详解

struct iovec {
    void *iov_base;    /* Starting address */
    size_t iov_len;    /* Number of bytes to transfer */
};
ssize_t readv(int fd, const struct iovec*iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
