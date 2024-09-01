#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <cassert>

#include "utils.h"
#include "constants.h"

/**
 * Receives a single connection fd and returns a single response
 */
int32_t one_request(int connfd)
{
    // Reading header
    char rbuf[4 + K_MAX_MSG + 1]; // 4 bytes
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err) {
        std::string msg = errno == 0 ? "EOF" : "read() error";
        std::cerr << msg << std::endl;
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4); // little endian
    if (len > K_MAX_MSG) {
        std::cerr << "too long" << std::endl;
        return -1;
    }

    // request body
    err = read_full(connfd, &rbuf[4], len);
    if (err) {
        std::cerr << "read() error" << std::endl;
        return -1;
    }

    // do something here
    rbuf[4 + len] = '\0';
    std::cout << "client says: " << &rbuf[4] << std::endl;

    std::string reply = "received: ";
    reply.append(&rbuf[4], len);
    char wbuf[4 + reply.size() + 1]; // can use sizeof as well but without the +1
    len = reply.size();
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply.c_str(), len);
    return write_all(connfd, wbuf, 4 + len);

    return 0;
}
