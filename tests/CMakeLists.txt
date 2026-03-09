cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_memory_store test_memory_store.cpp)
target_link_libraries(test_memory_store GTest::gtest_main ecatekv_core)

add_executable(test_protocol test_protocol.cpp)
target_link_libraries(test_protocol GTest::gtest_main ecatekv_core)

add_executable(test_wal test_wal.cpp)
target_link_libraries(test_wal GTest::gtest_main ecatekv_core)

include(GoogleTest)
gtest_discover_tests(test_memory_store)
gtest_discover_tests(test_protocol)
gtest_discover_tests(test_wal)
