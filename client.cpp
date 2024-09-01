#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

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
    std::vector<std::string> requests = {"hello1", "hello2", "hello3"};
    int num_count = 3;
    for (int count = 0; count < num_count; count++)
    {
        for (std::string req : requests)
        {
            if (send_req(fd, req.c_str()))
                break;
        }

        // Response
        for (int i = 0; i < requests.size(); i++)
        {
            if (read_res(fd))
                break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    close(fd);
    return 0;
}
