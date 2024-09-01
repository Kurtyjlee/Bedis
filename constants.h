#include <unistd.h>
#include <cstdint>

#ifndef CONSTANTS_H
#define CONSTANTS_H

const size_t K_MAX_MSG = 4096;

enum
{
    STATE_REQ = 0,
    STATE_RES = 1,
    STATE_END = 3,
};

struct Conn
{
    int fd = -1;
    uint32_t state = 0;

    // Buffer for reading for non-blocking mode
    size_t rbuf_size = 0;
    uint8_t rbuf[4 + K_MAX_MSG];

    // Buffer for writing for non-blocking mode
    size_t wbuf_size = 0;
    size_t wbuf_sent = 0;
    uint8_t wbuf[4 + K_MAX_MSG];
};

#endif
