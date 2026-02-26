#pragma once
#include "http_request.h"

class HttpParser {
    
    public:
        HttpRequest parse(const std::string& raw);
};