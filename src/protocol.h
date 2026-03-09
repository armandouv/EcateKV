#ifndef ECATEKV_SRC_PROTOCOL_H_
#define ECATEKV_SRC_PROTOCOL_H_
#include <cstdint>
#include <vector>
#include <string>

namespace ecatekv {

/**
 * @brief Represents the logical operation of the request/response.
 */
enum class Opcode : uint8_t {
    GET = 1,
    SET = 2,
    DEL = 3
};

/**
 * @brief Represents the success state of an operation.
 */
enum class Status : uint8_t {
    OK = 0,
    ERR = 1
};

/**
 * @brief Fixed 12-byte binary protocol header.
 */
struct Header {
    uint16_t magic = 0xEC47; // Arbitrary magic bytes
    Opcode opcode = Opcode::GET;
    Status status = Status::OK;
    uint32_t key_len = 0;
    uint32_t value_len = 0;
};

// Size of the header: 2 + 1 + 1 + 4 + 4 = 12 bytes
constexpr size_t HEADER_SIZE = 12;

/**
 * @brief A fully parsed structural message bridging network data and engine logic.
 */
struct Message {
    Header header;
    std::vector<uint8_t> key;
    std::vector<uint8_t> value;
};

/**
 * @brief Static utility class for enforcing binary wire formats.
 * 
 * Handles serialization to and from byte buffers, explicitly enforcing Network
 * Byte Order (Big-Endian) to ensure cross-platform compatibility.
 */
class Protocol {
public:
    /**
     * @brief Serializes a structured Message into a standard byte buffer.
     * @param msg The message to serialize.
     * @return the raw bytes ready to be written to a socket or file.
     */
    static std::vector<uint8_t> Serialize(const Message& msg);
    
    /**
     * @brief Attempts to parse a message from an incoming byte buffer stream.
     * @param buffer The dynamically expanding read buffer from a client connection.
     * @param out_msg The target message struct to populate.
     * @return true if a full message was parsed (and those bytes are consumed from the buffer).
     */
    static bool Parse(std::vector<uint8_t>& buffer, Message& out_msg);
};

} // namespace ecatekv


#endif  // ECATEKV_SRC_PROTOCOL_H_
