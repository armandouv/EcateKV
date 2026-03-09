#include "ecatekv/memory_store.h"
#include <mutex>
#include <iostream>

namespace ecatekv {

MemoryStore::MemoryStore(const std::string& wal_path) {
    if (!wal_path.empty()) {
        wal_ = std::make_unique<WriteAheadLog>(wal_path);
        auto msgs = wal_->Recover();
        for (const auto& msg : msgs) {
            std::string key(msg.key.begin(), msg.key.end());
            if (msg.header.opcode == Opcode::SET) {
                data_[key] = msg.value;
            } else if (msg.header.opcode == Opcode::DEL) {
                data_.erase(key);
            }
        }
        std::cout << "Recovered " << msgs.size() << " operations from WAL." << std::endl;
    }
}

std::optional<std::vector<uint8_t>> MemoryStore::Get(const std::string& key) {
    std::shared_lock lock(mutex_); // Read lock
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool MemoryStore::Set(const std::string& key, const std::vector<uint8_t>& value) {
    if (wal_) {
        Message msg;
        msg.header.opcode = Opcode::SET;
        msg.header.key_len = key.size();
        msg.header.value_len = value.size();
        msg.key = std::vector<uint8_t>(key.begin(), key.end());
        msg.value = value;
        if (!wal_->Append(msg)) return false;
    }

    std::unique_lock lock(mutex_); // Write lock
    data_[key] = value;
    return true;
}

bool MemoryStore::Del(const std::string& key) {
    if (wal_) {
        Message msg;
        msg.header.opcode = Opcode::DEL;
        msg.header.key_len = key.size();
        msg.header.value_len = 0;
        msg.key = std::vector<uint8_t>(key.begin(), key.end());
        if (!wal_->Append(msg)) return false;
    }

    std::unique_lock lock(mutex_); // Write lock
    return data_.erase(key) > 0;
}

} // namespace ecatekv
