cmake_minimum_required(VERSION 3.20)
project(http_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(http_server
    src/main.cpp
    src/server.cpp
    src/http_parser.cpp
    src/http_response.cpp
)

target_include_directories(http_server PRIVATE include)
