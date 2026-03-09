#include "ecatekv/server.h"
#include "ecatekv/memory_store.h"
#include <iostream>
#include <memory>
#include <csignal>

using namespace ecatekv;

std::unique_ptr<Server> g_server;

void HandleSigInt(int) {
    if (g_server) {
        std::cout << "\nStopping server gracefully..." << std::endl;
        g_server->Stop();
    }
}

int main(int argc, char** argv) {
    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    std::signal(SIGINT, HandleSigInt);

    try {
        std::string wal_path = "ecatekv.log";
        auto store = std::make_shared<MemoryStore>(wal_path);
        
        g_server = std::make_unique<Server>("0.0.0.0", port, store);
        g_server->Run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
