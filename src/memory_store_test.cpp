#include <gtest/gtest.h>
#include "memory_store.h"
#include <thread>
#include <vector>

TEST(MemoryStoreTest, BasicOperations) {
    ecatekv::MemoryStore store;

    // GET non-existent
    auto val = store.Get("key1");
    EXPECT_FALSE(val.has_value());

    // SET
    std::vector<uint8_t> data1 = {1, 2, 3};
    EXPECT_TRUE(store.Set("key1", data1));

    // GET existing
    val = store.Get("key1");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), data1);

    // DEL 
    EXPECT_TRUE(store.Del("key1"));
    EXPECT_FALSE(store.Get("key1").has_value());

    // DEL non-existent
    EXPECT_FALSE(store.Del("key1"));
}

TEST(MemoryStoreTest, ThreadSafety) {
    ecatekv::MemoryStore store;
    
    // Concurrent writers
    auto writer = [&store](int id) {
        for (int i = 0; i < 1000; ++i) {
            std::string key = "key_" + std::to_string(id) + "_" + std::to_string(i);
            store.Set(key, {static_cast<uint8_t>(id)});
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(writer, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Verify all keys were written
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 1000; ++j) {
            std::string key = "key_" + std::to_string(i) + "_" + std::to_string(j);
            auto val = store.Get(key);
            ASSERT_TRUE(val.has_value());
            EXPECT_EQ(val.value()[0], i);
        }
    }
}

TEST(MemoryStoreTest, WalRecovery) {
    std::string test_wal_file = "test_memory_store_wal.log";
    remove(test_wal_file.c_str());

    {
        // First instance, create some data
        ecatekv::MemoryStore store1(test_wal_file);
        store1.Set("k1", {'v', '1'});
        store1.Set("k2", {'v', '2'});
        store1.Set("k3", {'v', '3'});
        store1.Del("k2"); // Delete one to verify deletes are replayed
    }

    {
        // Second instance, recover data
        ecatekv::MemoryStore store2(test_wal_file);
        
        auto val1 = store2.Get("k1");
        ASSERT_TRUE(val1.has_value());
        EXPECT_EQ(val1.value(), std::vector<uint8_t>({'v', '1'}));
        
        // k2 should be deleted
        auto val2 = store2.Get("k2");
        EXPECT_FALSE(val2.has_value());
        
        auto val3 = store2.Get("k3");
        ASSERT_TRUE(val3.has_value());
        EXPECT_EQ(val3.value(), std::vector<uint8_t>({'v', '3'}));
    }

    remove(test_wal_file.c_str());
}
