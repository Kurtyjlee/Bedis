#include <cstdint>
#include <stddef.h>

void die(const char *);
int32_t read_full(int, char *, size_t);
int32_t write_all(int, const char *, size_t);
void fd_set_nb(int);
