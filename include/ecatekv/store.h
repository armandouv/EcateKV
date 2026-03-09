#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace ecatekv {

/**
 * @brief Abstract interface representing a Key-Value storage engine.
 * 
 * Provides the fundamental operations expected from a KV store.
 * Implementations must ensure thread-safety if accessed concurrently.
 */
class Store {
public:
    virtual ~Store() = default;

    /**
     * @brief Retrieves a value associated with the given key.
     * @param key The string key to look up.
     * @return std::optional containing the byte vector if found, std::nullopt otherwise.
     */
    virtual std::optional<std::vector<uint8_t>> Get(const std::string& key) = 0;

    /**
     * @brief Inserts or updates a key-value pair in the store.
     * @param key The string key.
     * @param value The raw byte vector containing the value.
     * @return true if the operation was successful, false otherwise.
     */
    virtual bool Set(const std::string& key, const std::vector<uint8_t>& value) = 0;

    /**
     * @brief Removes a key-value pair from the store.
     * @param key The string key to remove.
     * @return true if the key was found and deleted, false if it did not exist.
     */
    virtual bool Del(const std::string& key) = 0;
};

} // namespace ecatekv
