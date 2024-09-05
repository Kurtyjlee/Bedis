#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <cassert>

#include "constants.h"

static std::map<std::string, std::string> g_map;

/**
 * Get item from the database
 */
static uint32_t do_get(const std::vector<std::string> &cmd, uint8_t *res, uint32_t *reslen)
{
    if (!g_map.count(cmd[1]))
    {
        return RES_NX;
    }

    std::string &val = g_map[cmd[1]];
    assert(val.size() <= K_MAX_MSG);
    memcpy(res, val.data(), val.size());
    *reslen = (uint32_t)val.size();
    return RES_OK;
}

/**
 * Will set an item and overwrite if the item is already there
 */
static uint32_t do_set(const std::vector<std::string> &cmd, uint8_t *res, uint32_t *reslen) {
    (void)res;
    (void)reslen;
    g_map[cmd[1]] = cmd[2];
    return RES_OK;
}

/**
 * Delete item from the database
 */
static uint32_t do_del(const std::vector<std::string> &cmd, uint8_t *res, uint32_t *reslen) {
    (void)res;
    (void)reslen;
    g_map.erase(cmd[1]);
    return RES_OK; 
}

/**
 * Parse the req into a vector
 */
static int32_t parse_req(const uint8_t *data, size_t len, std::vector<std::string> &out)
{
    if (len < 4)
        return -1;

    // Get the length of the command
    uint32_t n = 0;
    memcpy(&n, &data[0], 4);

    if (n > K_MAX_MSG)
        return -1;

    // Getting the command
    size_t pos = 4;
    while (n--)
    {
        if (pos + 4 > len)
            return -1;

        uint32_t sz = 0;
        memcpy(&sz, &data[pos], 4);
        if (pos + 4 + sz > len)
            return -1;

        out.push_back(std::string((char *)&data[pos + 4], sz));
        pos += 4 + sz;
    }
    
    // If there is trailing garbage
    if (pos != len)
    {
        return -1;
    }
    return 0;
}
/**
 * Performs the request
 */
int32_t do_request(const uint8_t *req, uint32_t reqlen, uint32_t *rescode, uint8_t *res, uint32_t *reslen)
{
    std::vector<std::string> cmd;
    if (0 != parse_req(req, reqlen, cmd))
    {
        std::cout << "bad req" << std::endl;
        return -1;
    }

    std::cout << "client says: ";
    for (std::string c: cmd) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    if (cmd.size() == 2 && !strcmp(cmd[0].c_str(), "get"))
    {
        *rescode = do_get(cmd, res, reslen);
    }
    else if (cmd.size() == 3 && !strcmp(cmd[0].c_str(), "set"))
    {
        *rescode = do_set(cmd, res, reslen);
    }
    else if (cmd.size() == 2 && !strcmp(cmd[0].c_str(), "del"))
    {
        *rescode = do_del(cmd, res, reslen);
    }
    // cmd not recognised
    else
    {
        *rescode = RES_ERR;
        const char *msg = "Unknown cmd";
        strcpy((char *)res, msg);
        *reslen = strlen(msg);
    }
    return 0;
}
