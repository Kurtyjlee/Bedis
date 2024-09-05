#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <poll.h>

#include "utils.h"
#include "constants.h"
#include "eventloop.h"

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

    // map of all client connections, keyed by fd. 
    // That is, the index is the fd and the item is the Conn
    std::vector<Conn *> fd2conn;

    fd_set_nb(fd);

    // event loop
    std::vector<struct pollfd> poll_args;
    while (true)
    {
        // reset
        poll_args.clear();

        // listening fd
        struct pollfd pfd = {fd, POLLIN, 0};
        poll_args.push_back(pfd);

        // connection fds
        for (Conn *conn : fd2conn)
        {
            if (!conn)
                continue;

            struct pollfd pfd = {};
            pfd.fd = conn->fd;
            pfd.events = (conn->state == STATE_REQ) ? POLLIN : POLLOUT;
            pfd.events = pfd.events | POLLERR;
            poll_args.push_back(pfd);
        }

        // poll for active fds with a timeout
        // TODO: try out epollq
        int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
        if (rv < 0)
            die("poll");

        // process active connections, first fd is the listening fd
        for (size_t i = 1; i < poll_args.size(); ++i)
        {
            if (!poll_args[i].revents) continue;

            Conn *conn = fd2conn[poll_args[i].fd];
            connection_io(conn);

            if (conn->state != STATE_END) continue;

            fd2conn[conn->fd] = NULL;
            (void)close(conn->fd);
            free(conn);
        }
        if (poll_args[0].revents) {
            (void)accept_new_conn(fd2conn, fd);
        }
    }
    return 0;
}
