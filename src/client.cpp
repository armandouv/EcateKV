#include "ecatekv/protocol.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <chrono>

using namespace ecatekv;

int Connect(const std::string& host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

Message SendRecv(int sock, const Message& req) {
    std::vector<uint8_t> req_buf = Protocol::Serialize(req);
    write(sock, req_buf.data(), req_buf.size());

    std::vector<uint8_t> resp_buf;
    uint8_t chunk[1024];

    Message resp;
    while (true) {
        ssize_t n = read(sock, chunk, sizeof(chunk));
        if (n <= 0) break;
        resp_buf.insert(resp_buf.end(), chunk, chunk + n);
        
        if (Protocol::Parse(resp_buf, resp)) {
            break;
        }
    }
    return resp;
}

int main() {
    std::string host = "127.0.0.1";
    int port = 8080;

    int sock = Connect(host, port);
    if (sock < 0) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        return 1;
    }

    std::cout << "Connected to EcateKV!" << std::endl;

    // Benchmark SET
    auto start = std::chrono::high_resolution_clock::now();
    int NUM_OPS = 10000;
    
    for (int i = 0; i < NUM_OPS; ++i) {
        Message req;
        req.header.opcode = Opcode::SET;
        
        std::string key = "key" + std::to_string(i);
        std::string val = "val" + std::to_string(i);
        
        req.header.key_len = key.size();
        req.header.value_len = val.size();
        req.key = std::vector<uint8_t>(key.begin(), key.end());
        req.value = std::vector<uint8_t>(val.begin(), val.end());
        
        Message resp = SendRecv(sock, req);
        if (resp.header.status != Status::OK) {
            std::cerr << "SET failed for " << key << std::endl;
            break;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Performed " << NUM_OPS << " SET operations in " << ms << " ms. (" 
              << (NUM_OPS * 1000.0 / ms) << " ops/sec)" << std::endl;


    // GET
    Message req;
    req.header.opcode = Opcode::GET;
    std::string key = "key100";
    req.header.key_len = key.size();
    req.key = std::vector<uint8_t>(key.begin(), key.end());
    
    Message resp = SendRecv(sock, req);
    if (resp.header.status == Status::OK) {
        std::string val(resp.value.begin(), resp.value.end());
        std::cout << "GET " << key << " -> " << val << std::endl;
    } else {
        std::cout << "GET " << key << " failed." << std::endl;
    }

    close(sock);
    return 0;
}
