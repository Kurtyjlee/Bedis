#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <cassert>

#include "utils.h"
#include "constants.h"

/**
 * Sent queries to the fd with text as the payload and receive a response from the server
 */
int32_t query(int fd, const char *text) {
    uint32_t len = (uint32_t)strlen(text);
    if (len > K_MAX_MSG) {
        return -1;
    }

    // Request to server
    char wbuf[4 + K_MAX_MSG];
    memcpy(wbuf, &len, 4); // little endian
    memcpy(&wbuf[4], text, len);
    int32_t err = write_all(fd, wbuf, 4 + len);
    if (err) 
        return err;
    
    // Response from server
    // header: 4bytes
    char rbuf[4 + K_MAX_MSG + 1];
    errno = 0;
    err = read_full(fd, rbuf, 4);
    if (err) {
        std::string msg = errno == 0 ? "EOF" : "read() errr";
        std::cerr << msg << std::endl;
        return err;
    }
    memcpy(&len, rbuf, 4);
    if (len > K_MAX_MSG) {
        std::cerr << "too long" << std::endl;
        return -1;
    }

    // Response body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        std::cerr << "read() error" << std::endl;
        return err;
    }

    // simulate
    rbuf[4 + len] = '\0';
    std::cout << "server says: " << &rbuf[4] << std::endl;
    return 0;
}
