#pragma once

#include "protocol.h"
#include <vector>
#include <cstdint>
#include <unistd.h>

namespace ecatekv {

/**
 * @brief Represents an active client connection handled by the server.
 * 
 * Encapsulates the socket file descriptor and asynchronous state buffers.
 * Maintains vector buffers for reading partial requests and writing partial
 * responses in a completely non-blocking manner.
 */
class Connection {
public:
    int fd = -1; // Socket file descriptor
    std::vector<uint8_t> read_buffer; // Accumulates incoming network bytes
    std::vector<uint8_t> write_buffer; // Accumulates bytes waiting to be sent
    
    /**
     * @brief Flag denoting that the server loop should gracefully close this client.
     */
    bool want_close = false;

    explicit Connection(int fd) : fd(fd) {}
    ~Connection() {
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
    }
    
    // Non-copyable to safely manage socket lifecycle via RAII
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};

} // namespace ecatekv
