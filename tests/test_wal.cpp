#include <gtest/gtest.h>
#include "ecatekv/wal.h"

using namespace ecatekv;

TEST(WALTest, AppendAndRecover) {
    std::string test_file = "test_wal.log";
    remove(test_file.c_str());

    {
        WriteAheadLog wal(test_file);
        
        Message msg1;
        msg1.header.magic = 0xEC47;
        msg1.header.opcode = Opcode::SET;
        msg1.header.status = Status::OK;
        msg1.key = {'k', '1'};
        msg1.value = {'v', '1'};
        msg1.header.key_len = 2;
        msg1.header.value_len = 2;
        
        EXPECT_TRUE(wal.Append(msg1));
        
        Message msg2;
        msg2.header.magic = 0xEC47;
        msg2.header.opcode = Opcode::DEL;
        msg2.header.status = Status::OK;
        msg2.key = {'k', '2'};
        msg2.header.key_len = 2;
        msg2.header.value_len = 0;
        
        EXPECT_TRUE(wal.Append(msg2));
    }

    {
        WriteAheadLog wal2(test_file);
        auto msgs = wal2.Recover();
        
        ASSERT_EQ(msgs.size(), 2);
        EXPECT_EQ(msgs[0].header.opcode, Opcode::SET);
        EXPECT_EQ(msgs[0].key.size(), 2);
        
        EXPECT_EQ(msgs[1].header.opcode, Opcode::DEL);
        EXPECT_EQ(msgs[1].key.size(), 2);
    }

    remove(test_file.c_str());
}
