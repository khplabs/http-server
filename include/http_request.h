#pragma once
#include <string>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    // O(1) constant time lookup by header name
    std::unordered_map<std::string, std::string> headers;
};