#include "ecatekv/protocol.h"
#include <cstring>
#include <arpa/inet.h>

namespace ecatekv {

std::vector<uint8_t> Protocol::Serialize(const Message& msg) {
    size_t total_size = HEADER_SIZE + msg.header.key_len + msg.header.value_len;
    std::vector<uint8_t> buf(total_size);
    
    size_t offset = 0;
    
    uint16_t magic_net = htons(msg.header.magic);
    std::memcpy(buf.data() + offset, &magic_net, 2);
    offset += 2;
    
    buf[offset++] = static_cast<uint8_t>(msg.header.opcode);
    buf[offset++] = static_cast<uint8_t>(msg.header.status);
    
    uint32_t key_len_net = htonl(msg.header.key_len);
    std::memcpy(buf.data() + offset, &key_len_net, 4);
    offset += 4;
    
    uint32_t val_len_net = htonl(msg.header.value_len);
    std::memcpy(buf.data() + offset, &val_len_net, 4);
    offset += 4;
    
    if (msg.header.key_len > 0) {
        std::memcpy(buf.data() + offset, msg.key.data(), msg.header.key_len);
        offset += msg.header.key_len;
    }
    
    if (msg.header.value_len > 0) {
        std::memcpy(buf.data() + offset, msg.value.data(), msg.header.value_len);
        offset += msg.header.value_len;
    }
    
    return buf;
}

bool Protocol::Parse(std::vector<uint8_t>& buffer, Message& out_msg) {
    if (buffer.size() < HEADER_SIZE) {
        return false;
    }
    
    size_t offset = 0;
    
    uint16_t magic_net;
    std::memcpy(&magic_net, buffer.data() + offset, 2);
    out_msg.header.magic = ntohs(magic_net);
    offset += 2;
    
    out_msg.header.opcode = static_cast<Opcode>(buffer[offset++]);
    out_msg.header.status = static_cast<Status>(buffer[offset++]);
    
    uint32_t key_len_net;
    std::memcpy(&key_len_net, buffer.data() + offset, 4);
    out_msg.header.key_len = ntohl(key_len_net);
    offset += 4;
    
    uint32_t val_len_net;
    std::memcpy(&val_len_net, buffer.data() + offset, 4);
    out_msg.header.value_len = ntohl(val_len_net);
    offset += 4;
    
    size_t total_msg_len = HEADER_SIZE + out_msg.header.key_len + out_msg.header.value_len;
    if (buffer.size() < total_msg_len) {
        return false; // Not enough data yet
    }
    
    if (out_msg.header.key_len > 0) {
        out_msg.key.resize(out_msg.header.key_len);
        std::memcpy(out_msg.key.data(), buffer.data() + offset, out_msg.header.key_len);
        offset += out_msg.header.key_len;
    } else {
        out_msg.key.clear();
    }
    
    if (out_msg.header.value_len > 0) {
        out_msg.value.resize(out_msg.header.value_len);
        std::memcpy(out_msg.value.data(), buffer.data() + offset, out_msg.header.value_len);
        offset += out_msg.header.value_len;
    } else {
        out_msg.value.clear();
    }
    
    // Remove parsed message from buffer
    buffer.erase(buffer.begin(), buffer.begin() + total_msg_len);
    
    return true;
}

} // namespace ecatekv
