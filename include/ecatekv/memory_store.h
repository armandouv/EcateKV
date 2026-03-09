#pragma once

#include "store.h"
#include "wal.h"
#include <unordered_map>
#include <shared_mutex>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <memory>

namespace ecatekv {

/**
 * @brief An in-memory concurrent Key-Value storage engine backed by a Write-Ahead Log.
 * 
 * Utilizes std::unordered_map for O(1) average time complexity operations.
 * Thread-safety is achieved using a standard Reader-Writer lock (std::shared_mutex),
 * allowing multiple concurrent readers or exclusive writer access.
 * If a WAL path is provided, it automatically recovers state on initialization and 
 * persists all mutations dynamically.
 */
class MemoryStore : public Store {
public:
    /**
     * @brief Constructs the MemoryStore, optionally bootstrapping from a WAL file.
     * @param wal_path The filepath to the Write-Ahead Log. If empty, persistence is disabled.
     */
    explicit MemoryStore(const std::string& wal_path = "");
    ~MemoryStore() override = default;

    /**
     * @brief Retrieves a value by key. Acquires a shared (read) lock.
     */
    std::optional<std::vector<uint8_t>> Get(const std::string& key) override;

    /**
     * @brief Sets a value for a key. Acquires a unique (write) lock. 
     * Appends to the WAL before updating memory if WAL is active.
     */
    bool Set(const std::string& key, const std::vector<uint8_t>& value) override;

    /**
     * @brief Deletes a key. Acquires a unique (write) lock.
     * Appends to the WAL before mutating memory if WAL is active.
     */
    bool Del(const std::string& key) override;

private:
    std::unordered_map<std::string, std::vector<uint8_t>> data_;
    mutable std::shared_mutex mutex_;
    std::unique_ptr<WriteAheadLog> wal_;
};

} // namespace ecatekv
