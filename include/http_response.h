#pragma once
#include <string>

class HttpResponse {
    public:
        std::string build(int status_code, const std::string& body);

    private:
        std::string status_text(int code);
};