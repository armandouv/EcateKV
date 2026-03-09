#include "ecatekv/server.h"
#include "ecatekv/socket_utils.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

namespace ecatekv {

constexpr int MAX_EVENTS = 1024;

Server::Server(const std::string& host, int port, std::shared_ptr<Store> store)
    : host_(host), port_(port), store_(store) {
    
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        throw std::runtime_error("Failed to create server socket");
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        throw std::runtime_error("setsockopt failed");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host_.c_str());
    addr.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind to port " + std::to_string(port_));
    }

    if (listen(server_fd_, SOMAXCONN) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    MakeNonBlocking(server_fd_);

    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("Failed to create epoll instance");
    }

    EpollCtl(server_fd_, EPOLL_CTL_ADD, EPOLLIN);
}

Server::~Server() {
    Stop();
    if (server_fd_ != -1) close(server_fd_);
    if (epoll_fd_ != -1) close(epoll_fd_);
}

void Server::EpollCtl(int fd, int op, uint32_t events, void* ptr) {
    epoll_event ev{};
    ev.events = events;
    if (ptr) {
        ev.data.ptr = ptr;
    } else {
        ev.data.fd = fd;
    }

    if (epoll_ctl(epoll_fd_, op, fd, &ev) < 0) {
        std::cerr << "epoll_ctl failed for fd: " << fd << std::endl;
    }
}

void Server::Stop() {
    running_ = false;
}

void Server::Run() {
    running_ = true;
    epoll_event events[MAX_EVENTS];

    std::cout << "Server starting on " << host_ << ":" << port_ << std::endl;

    while (running_) {
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == server_fd_) {
                AcceptConnections();
            } else {
                auto* conn = static_cast<Connection*>(events[i].data.ptr);
                if (events[i].events & EPOLLIN) {
                    HandleRead(conn);
                }
                if (events[i].events & EPOLLOUT) {
                    HandleWrite(conn);
                }
                if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || conn->want_close) {
                    CloseConnection(conn->fd);
                }
            }
        }
    }
}

void Server::AcceptConnections() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // All connections accepted for now
            } else {
                std::cerr << "accept failed" << std::endl;
                break;
            }
        }

        MakeNonBlocking(client_fd);

        auto conn = std::make_unique<Connection>(client_fd);
        auto* conn_ptr = conn.get();
        connections_[client_fd] = std::move(conn);

        // Edge-triggered, read/write
        EpollCtl(client_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLET, conn_ptr);
    }
}

void Server::HandleRead(Connection* conn) {
    while (true) {
        uint8_t buf[4096];
        ssize_t n = read(conn->fd, buf, sizeof(buf));
        
        if (n > 0) {
            conn->read_buffer.insert(conn->read_buffer.end(), buf, buf + n);
            
            // Try to parse as many messages as possible
            Message msg;
            while (Protocol::Parse(conn->read_buffer, msg)) {
                ProcessMessage(conn, msg);
            }
        } else if (n == 0) {
            conn->want_close = true; // Client disconnected
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // Nothing left to read
            }
            conn->want_close = true; // Error
            break;
        }
    }
}

void Server::HandleWrite(Connection* conn) {
    while (!conn->write_buffer.empty()) {
        ssize_t n = write(conn->fd, conn->write_buffer.data(), conn->write_buffer.size());
        
        if (n > 0) {
            conn->write_buffer.erase(conn->write_buffer.begin(), conn->write_buffer.begin() + n);
        } else if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // Socket buffer full
            }
            conn->want_close = true;
            break;
        }
    }
}

void Server::ProcessMessage(Connection* conn, const Message& req) {
    Message resp;
    resp.header.magic = 0xEC47;
    resp.header.opcode = req.header.opcode;
    
    if (req.header.opcode == Opcode::GET) {
        std::string key(req.key.begin(), req.key.end());
        auto val = store_->Get(key);
        if (val.has_value()) {
            resp.header.status = Status::OK;
            resp.value = val.value();
            resp.header.value_len = resp.value.size();
        } else {
            resp.header.status = Status::ERR; // Not found
        }
    } else if (req.header.opcode == Opcode::SET) {
        std::string key(req.key.begin(), req.key.end());
        if (store_->Set(key, req.value)) {
            resp.header.status = Status::OK;
        } else {
            resp.header.status = Status::ERR;
        }
    } else if (req.header.opcode == Opcode::DEL) {
        std::string key(req.key.begin(), req.key.end());
        if (store_->Del(key)) {
            resp.header.status = Status::OK;
        } else {
            resp.header.status = Status::ERR;
        }
    } else {
        resp.header.status = Status::ERR; // Unknown opcode
    }
    
    std::vector<uint8_t> payload = Protocol::Serialize(resp);
    conn->write_buffer.insert(conn->write_buffer.end(), payload.begin(), payload.end());
    
    // Attempt to write immediately if possible, HandleWrite will pick up any remaining bytes
    HandleWrite(conn);
}

void Server::CloseConnection(int fd) {
    EpollCtl(fd, EPOLL_CTL_DEL, 0);
    connections_.erase(fd);
    // Connection destructor will close the fd
}

} // namespace ecatekv
