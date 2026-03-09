cmake_minimum_required(VERSION 3.20)
project(EcateKV VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -pthread")



add_library(ecatekv_core
    src/memory_store.cpp
    src/protocol.cpp
    src/server.cpp
    src/wal.cpp
)

target_include_directories(ecatekv_core PUBLIC src)
target_link_libraries(ecatekv_core pthread)

add_executable(ecatekv_server src/main.cpp)
target_link_libraries(ecatekv_server ecatekv_core)

add_executable(ecatekv_client src/client.cpp)
target_link_libraries(ecatekv_client ecatekv_core)

# Testing
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(memory_store_test src/memory_store_test.cpp)
target_link_libraries(memory_store_test GTest::gtest_main ecatekv_core)

add_executable(protocol_test src/protocol_test.cpp)
target_link_libraries(protocol_test GTest::gtest_main ecatekv_core)

add_executable(wal_test src/wal_test.cpp)
target_link_libraries(wal_test GTest::gtest_main ecatekv_core)

include(GoogleTest)
gtest_discover_tests(memory_store_test)
gtest_discover_tests(protocol_test)
gtest_discover_tests(wal_test)
