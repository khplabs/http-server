cmake_minimum_required(VERSION 3.20)
project(http_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(http_server
    src/main.cpp
    src/server.cpp
    src/http_parser.cpp
    src/http_response.cpp
    src/file_handler.cpp
)

target_include_directories(http_server PRIVATE include)
target_link_options(http_server PRIVATE -static-libstdc++ -static-libgcc)

find_package(Threads REQUIRED)
target_link_libraries(http_server PRIVATE Threads::Threads)