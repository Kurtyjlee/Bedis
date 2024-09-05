#include <vector>
#include <iostream>
#include <netinet/ip.h>
#include <assert.h>

#include "constants.h"
#include "utils.h"
#include "commands.h"

/**
 * Insert the connection into the fd map
 */
void conn_put(std::vector<Conn *> &fd2conn, struct Conn *conn)
{
    if (fd2conn.size() <= (size_t)conn->fd)
    {
        fd2conn.resize(conn->fd + 1);
    }
    fd2conn[conn->fd] = conn;
}

/**
 * Accepts the connection and add the struct object into fd2conn
 */
int32_t accept_new_conn(std::vector<Conn *> &fd2conn, int fd)
{
    // Accept the connection
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
    if (connfd < 0)
    {
        std::cerr << "accept() error" << std::endl;
        return -1;
    }

    fd_set_nb(connfd);

    // Conn struct
    struct Conn *conn = (struct Conn *)malloc(sizeof(struct Conn));
    if (!conn)
    {
        std::cerr << "Unable to create Conn struct" << std::endl;
        close(connfd);
        return -1;
    }
    conn->fd = connfd;
    conn->state = STATE_REQ;

    // rbuf_size, wbuf_size and wbuf_sent are 0 by default
    conn_put(fd2conn, conn);

    return 0;
}

/**
 * Will transit the state back to STATE_REQ once the flushing is done
 */
bool try_flush_buffer(Conn *conn)
{
    ssize_t rv = 0;

    // Re-write when get signal interrupt
    do
    {
        size_t remain = conn->wbuf_size - conn->wbuf_sent;
        rv = write(conn->fd, &conn->wbuf[conn->wbuf_sent], remain);
    } while (rv < 0 && errno == EINTR);

    // Stop when get EAGAIN
    if (rv < 0 && errno == EAGAIN)
        return false;

    if (rv < 0)
    {
        std::cerr << "write() error" << std::endl;
        conn->state = STATE_END;
        return false;
    }

    conn->wbuf_sent += (size_t)rv;
    assert(conn->wbuf_sent <= conn->wbuf_size);
    // Response full sent
    if (conn->wbuf_sent == conn->wbuf_size)
    {
        conn->state = STATE_REQ;
        conn->wbuf_sent = 0;
        conn->wbuf_size = 0;
        return false;
    }

    // Need to clear all the data in wbuf
    return true;
}

/**
 * Will flush the write buffer till EAGAIN
 * TODO: Instead of writing for every response, buffer multiple response and flush together
 */
void state_res(Conn *conn)
{
    while (try_flush_buffer(conn))
    {
    };
}

/**
 * Takes 1 request from the buffer, generates a response then transits to STATE_RES
 * we have a request of
 * +------+-----+------+-----+------+-----+-----+------+
 * | nstr | len | str1 | len | str2 | ... | len | strn |
 * +------+-----+------+-----+------+-----+-----+------+
 * Where nstr and len are 32-bit integers
 */
bool try_one_request(Conn *conn)
{
    // try to parse a request from the buffer
    if (conn->rbuf_size < 4)
        return false;

    uint32_t len = 0;
    memcpy(&len, &conn->rbuf[0], 4);
    if (len > K_MAX_MSG)
    {
        msg("too long");
        conn->state = STATE_END;
        return false;
    }

    // Not enough data in the buffer
    if (4 + len > conn->rbuf_size)
        return false;

    // got one request, generate the response.
    uint32_t rescode = 0;
    uint32_t wlen = 0;
    int32_t err = do_request(
        &conn->rbuf[4], len, &rescode, &conn->wbuf[8], &wlen);
    if (err)
    {
        conn->state = STATE_END;
        return false;
    }

    wlen += 4;
    memcpy(&conn->wbuf[0], &wlen, 4);
    memcpy(&conn->wbuf[4], &rescode, 4);
    conn->wbuf_size = 4 + wlen;

    // remove the request from the buffer.
    // note: frequent memmove is inefficient.
    // TODO: Only do this before read operations
    size_t remain = conn->rbuf_size - 4 - len;
    if (remain)
        memmove(conn->rbuf, &conn->rbuf[4 + len], remain);

    conn->rbuf_size = remain;

    conn->state = STATE_RES;
    state_res(conn);

    // continue the outer loop if the request was fully processed
    return conn->state == STATE_REQ;
}

/**
 * Follows the pseudocode
 *
 * func do_somthing_to_client(fd):
 *      if should_read_from(fd):
 *          data = read_until_EAGAIN(fd)
 *          process_incoming_data(data)
 *
 */
bool try_fill_buffer(Conn *conn)
{
    assert(conn->rbuf_size < sizeof(conn->rbuf));
    ssize_t rv = 0;

    // Keep reading when hit EINTR
    // EINTR means syscall was interrupted by a signal
    do
    {
        size_t cap = sizeof(conn->rbuf) - conn->rbuf_size;
        rv = read(conn->fd, &conn->rbuf[conn->rbuf_size], cap);
    } while (rv < 0 && errno == EINTR); // EINTR means syscall was interrupted by a signal

    // Hit EAGAIN, means stop
    if (rv < 0 && errno == EAGAIN)
        return false;

    if (rv < 0)
    {
        std::cerr << "read() error" << std::endl;
        conn->state = STATE_END;
        return false;
    }

    if (rv == 0)
    {
        std::string msg = conn->rbuf_size > 0 ? "unexpected EOF" : "EOF";
        std::cerr << msg << std::endl;
        conn->state = STATE_END;
        return false;
    }

    conn->rbuf_size += (size_t)rv;
    assert(conn->rbuf_size <= sizeof(conn->rbuf));

    // Try to process one by one
    while (try_one_request(conn))
    {
    };
    return conn->state == STATE_REQ;
}

/**
 * State machine for reader
 *
 * Fills the read buffer with data.
 * The read buffer could be full before EAGAIN, so need to process the data immediately
 * after reading to clear buffer space, then try_fill_buffer is looped again
 */
void state_req(Conn *conn)
{
    while (try_fill_buffer(conn))
    {
    }
}

/**
 * State machine for client connections
 */
void connection_io(Conn *conn)
{
    switch (conn->state)
    {
    case STATE_REQ:
        state_req(conn);
        break;
    case STATE_RES:
        state_res(conn);
        break;
    default:
        assert(0); // not expected
    }
}
