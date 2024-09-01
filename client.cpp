#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <string>
#include <iostream>
#include "helper.h"

int main()
{
    // TCP socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        die("socket()");

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    // connect to socket
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv)
        die("connect");

    // write to server
    std::string msg = "hello";
    write(fd, msg.c_str(), msg.size());

    // read from server
    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
        die("read");

    std::cout << "server says: " << rbuf << std::endl;
    close(fd);
}
