#include "buffer.h"
#include <iostream>
#include <string>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Function to simulate reading from a file descriptor
void testReadFromFd(Buffer& buffer, int fd) {
    int errNo = 0;
    ssize_t n = buffer.readFd(fd, &errNo);
    if (n > 0) {
        std::cout << "Read " << n << " bytes from fd: " << fd << std::endl;
        std::cout << "Buffer content: " << buffer.retrieveAllToString() << std::endl;
    }
    else if (n == 0) {
        std::cout << "EOF reached for fd: " << fd << std::endl;
    }
    else {
        std::cerr << "Error reading from fd: " << fd << ", errno: " << errNo << std::endl;
    }
}

// Function to simulate writing to a file descriptor
void testWriteToFd(Buffer& buffer, int fd) {
    int errNo = 0;
    ssize_t n = buffer.writeFd(fd, &errNo);
    if (n >= 0) {
        std::cout << "Wrote " << n << " bytes to fd: " << fd << std::endl;
    }
    else {
        std::cerr << "Error writing to fd: " << fd << ", errno: " << errNo << std::endl;
    }
}

int main() {
    // Create a buffer and test various operations
    Buffer buffer;

    // Test append and retrieve
    buffer.append("Hello", 5);
    assert(buffer.readableBytes() == 5);
    std::cout << "Buffer content: " << buffer.retrieveAllToString() << std::endl;

    buffer.append("World", 5);
    buffer.append("!", 1);
    assert(buffer.readableBytes() == 6);
    std::cout << "Buffer content: " << buffer.retrieveAllToString() << std::endl;

    // Test from and to file descriptor
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
        return 1;
    }

    // Write some data to the write end of the pipe
    const char* testData = "Test data from fd";
    write(fds[1], testData, strlen(testData));

    // Read from the read end of the pipe
    testReadFromFd(buffer, fds[0]);

    // Write data to the write end of the pipe using buffer
    buffer.append("More test data", 14);
    testWriteToFd(buffer, fds[1]);

    // Read back the data
    testReadFromFd(buffer, fds[0]);

    // Clean up
    close(fds[0]);
    close(fds[1]);

    return 0;
}
