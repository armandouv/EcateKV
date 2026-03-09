#ifndef ECATEKV_SRC_WAL_H_
#define ECATEKV_SRC_WAL_H_

#include "protocol.h"
#include <string>
#include <vector>
#include <mutex>

namespace ecatekv {

/**
 * @brief Write-Ahead Log for persisting memory state to disk.
 * 
 * Ensures durability by writing serialized operations to a file immediately.
 * On server restart, the log can be replayed to reconstruct the dataset.
 */
class WriteAheadLog {
public:
    /**
     * @brief Opens or creates a WAL file.
     * @param filepath The path to the append-only log file.
     */
    explicit WriteAheadLog(const std::string& filepath);
    ~WriteAheadLog();

    /**
     * @brief Appends a serialized mutation (SET/DEL) to the log safely.
     * @param msg The message to serialize and write.
     * @return true if successfully written to disk, false otherwise.
     */
    bool Append(const Message& msg);
    
    /**
     * @brief Reads the entire log, parsing all historically appended messages.
     * @return A vector of messages representing the sequence of historical mutations.
     */
    std::vector<Message> Recover();

private:
    std::string filepath_;
    int fd_ = -1;
    off_t offset_ = 0;
    std::mutex mutex_;
};

} // namespace ecatekv


#endif  // ECATEKV_SRC_WAL_H_
