#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <iostream>

#include "helper.h"

static void do_something(int connfd)
{
    // read from client
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        fprintf(stderr, "%s\n", "read() error");
        return;
    }
    std::cout << "client says: " << rbuf << std::endl;

    // write to client
    std::string wbuf = "world";
    write(connfd, wbuf.c_str(), wbuf.size());
}

int main()
{
    // use AF_INET6 for ipv6 or dual-stack sockets
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;

    // ntohs and nothl will convert the numbers to big endian format
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // wildcard for 0.0.0.0

    // bind to socket
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
        die("bind()");

    // listen from socket. SOMACCONN is 128 by default
    rv = listen(fd, SOMAXCONN);
    if (rv)
        die("listen()");

    while (true)
    {
        // Accepting client connections
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int conndf = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (conndf < 0)
            continue;

        do_something(conndf);
        close(conndf);
    }

    return 0;
}
