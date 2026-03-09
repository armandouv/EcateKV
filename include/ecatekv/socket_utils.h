#pragma once

#include <fcntl.h>
#include <unistd.h>

namespace ecatekv {

inline bool MakeNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}

} // namespace ecatekv
