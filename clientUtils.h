#include <unistd.h>
#include <vector>
#include <string>

// int32_t query(int, const char *);
int32_t send_req(int fd, const std::vector<std::string> &);
int32_t read_res(int);
