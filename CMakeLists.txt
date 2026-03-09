cmake_minimum_required(VERSION 3.20)
project(EcateKV VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -pthread")

include_directories(include)

add_library(ecatekv_core
    src/memory_store.cpp
    src/protocol.cpp
    src/server.cpp
    src/wal.cpp
)

target_include_directories(ecatekv_core PUBLIC include)
target_link_libraries(ecatekv_core pthread)

add_executable(ecatekv_server src/main.cpp)
target_link_libraries(ecatekv_server ecatekv_core)

add_executable(ecatekv_client src/client.cpp)
target_link_libraries(ecatekv_client ecatekv_core)

enable_testing()
add_subdirectory(tests)
