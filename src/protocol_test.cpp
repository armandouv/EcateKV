#include <gtest/gtest.h>
#include "protocol.h"
#include <string>

using namespace ecatekv;

TEST(ProtocolTest, SerializeAndParse) {
    Message msg;
    msg.header.magic = 0xEC47;
    msg.header.opcode = Opcode::SET;
    msg.header.status = Status::OK;
    
    std::string key = "my_key";
    std::string val = "my_value";
    
    msg.header.key_len = key.size();
    msg.header.value_len = val.size();
    
    msg.key = std::vector<uint8_t>(key.begin(), key.end());
    msg.value = std::vector<uint8_t>(val.begin(), val.end());
    
    std::vector<uint8_t> buffer = Protocol::Serialize(msg);
    EXPECT_EQ(buffer.size(), HEADER_SIZE + key.size() + val.size());
    
    Message parsed_msg;
    bool success = Protocol::Parse(buffer, parsed_msg);
    EXPECT_TRUE(success);
    EXPECT_EQ(buffer.size(), 0); // Should be removed
    
    EXPECT_EQ(parsed_msg.header.magic, 0xEC47);
    EXPECT_EQ(parsed_msg.header.opcode, Opcode::SET);
    EXPECT_EQ(parsed_msg.header.status, Status::OK);
    EXPECT_EQ(parsed_msg.header.key_len, key.size());
    EXPECT_EQ(parsed_msg.header.value_len, val.size());
    
    std::string parsed_key(parsed_msg.key.begin(), parsed_msg.key.end());
    std::string parsed_val(parsed_msg.value.begin(), parsed_msg.value.end());
    
    EXPECT_EQ(parsed_key, key);
    EXPECT_EQ(parsed_val, val);
}

TEST(ProtocolTest, PartialParse) {
    Message msg;
    msg.header.magic = 0xEC47;
    msg.header.opcode = Opcode::GET;
    msg.header.status = Status::OK;
    msg.header.key_len = 4;
    msg.header.value_len = 0;
    msg.key = {'t', 'e', 's', 't'};
    
    std::vector<uint8_t> full_buffer = Protocol::Serialize(msg);
    
    // Test partial header
    std::vector<uint8_t> partial(full_buffer.begin(), full_buffer.begin() + 5);
    Message parsed_msg;
    EXPECT_FALSE(Protocol::Parse(partial, parsed_msg));
    
    // Test full header, but partial payload
    std::vector<uint8_t> almost_full(full_buffer.begin(), full_buffer.end() - 1);
    EXPECT_FALSE(Protocol::Parse(almost_full, parsed_msg));
    
    // Parses successfully once fully received
    EXPECT_TRUE(Protocol::Parse(full_buffer, parsed_msg));
}
