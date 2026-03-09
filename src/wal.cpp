#include "ecatekv/wal.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

namespace ecatekv {

WriteAheadLog::WriteAheadLog(const std::string& filepath) : filepath_(filepath) {
    fd_ = open(filepath_.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd_ < 0) {
        throw std::runtime_error("Failed to open WAL file: " + filepath_);
    }
    
    offset_ = lseek(fd_, 0, SEEK_END);
}

WriteAheadLog::~WriteAheadLog() {
    if (fd_ != -1) {
        close(fd_);
    }
}

bool WriteAheadLog::Append(const Message& msg) {
    std::vector<uint8_t> buffer = Protocol::Serialize(msg);
    if (buffer.empty()) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    
    ssize_t written = pwrite(fd_, buffer.data(), buffer.size(), offset_);
    if (written != static_cast<ssize_t>(buffer.size())) {
        return false;
    }
    
    offset_ += written;
    return true;
}

std::vector<Message> WriteAheadLog::Recover() {
    std::vector<Message> messages;
    lseek(fd_, 0, SEEK_SET); // Rewind
    
    std::vector<uint8_t> buffer;
    uint8_t chunk[4096];
    
    while (true) {
        ssize_t n = read(fd_, chunk, sizeof(chunk));
        if (n <= 0) break;
        
        buffer.insert(buffer.end(), chunk, chunk + n);
        
        Message msg;
        while (Protocol::Parse(buffer, msg)) {
            messages.push_back(std::move(msg));
        }
    }
    
    return messages;
}

} // namespace ecatekv
