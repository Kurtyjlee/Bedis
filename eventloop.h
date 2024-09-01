#include <vector>
#include "constants.h"

int32_t accept_new_conn(std::vector<Conn *> &, int);
void connection_io(Conn *);
