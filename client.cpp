#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <string>
#include <iostream>

#include "utils.h"
#include "clientUtils.h"

/**
 * Client to communicate with server using TCP
 */
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

    // Requests
    std::string requests[] = {"hello1", "hello2", "hello3"};
    for (std::string req : requests)
    {
        if (query(fd, req.c_str()))
            break;
    }
    close(fd);
}
