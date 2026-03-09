#pragma once

#include "connection.h"
#include "store.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace ecatekv {

/**
 * @brief High-performance asynchronous TCP Server.
 * 
 * Uses Linux epoll with Edge-Triggered (EPOLLET) configurations to multiplex
 * thousands of concurrent client connections over a single thread. Translates
 * binary network protocols to memory operations against a backing Store.
 */
class Server {
public:
    /**
     * @brief Initializes the server on the given network interface and port.
     * @param host IP address to bind to (e.g. "0.0.0.0").
     * @param port TCP port to listen on.
     * @param store The underlying Key-Value storage engine to handle requests.
     */
    Server(const std::string& host, int port, std::shared_ptr<Store> store);
    ~Server();

    /**
     * @brief Engages the infinite event loop handling epoll events.
     * Blocking call until Stop() is invoked.
     */
    void Run();

    /**
     * @brief Breaks the server event loop gracefully.
     */
    void Stop();

private:
    void AcceptConnections();
    void HandleRead(Connection* conn);
    void HandleWrite(Connection* conn);
    void ProcessMessage(Connection* conn, const Message& req);
    void EpollCtl(int fd, int op, uint32_t events, void* ptr = nullptr);
    void CloseConnection(int fd);

    std::string host_;
    int port_;
    std::shared_ptr<Store> store_;
    
    int server_fd_ = -1;
    int epoll_fd_ = -1;
    bool running_ = false;

    std::unordered_map<int, std::unique_ptr<Connection>> connections_;
};

} // namespace ecatekv
