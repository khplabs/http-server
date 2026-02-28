#include "http_response.h"

std::string HttpResponse::build(int status_code, const std::string& body, bool keep_alive) {
    // An HTTP/1.1 response looks like this:
    //
    // HTTP/1.1 200 OK\r\n
    // Content-Type: text/html\r\n
    // Content-Length: 13\r\n
    // \r\n
    // <actual body>
    //
    // The blank line (\r\n) separating headers from body is mandatory.
    // Without it the client won't know where headers end and body begins.

    std::string response;
    response += "HTTP/1.1 " + std::to_string(status_code) + " " + status_text(status_code) + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";

    if (keep_alive) {
        response += "Connection: keep-alive\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    response += "\r\n";
    response += body;
    return response;
}

std::string HttpResponse::status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 404: return "Not Found";
        default: return "Unknown";
    }
}