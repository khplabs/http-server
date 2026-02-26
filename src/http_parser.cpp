#include "http_parser.h"
#include <sstream>

HttpRequest HttpParser::parse(const std::string& raw) {
    HttpRequest request;

    // The first line of an HTTP request looks like:
    // GET /index.html HTTP/1.1
    // Everything after that is headers, which we ignore for now.
    std::istringstream stream(raw);
    std::string first_line;
    std::getline(stream, first_line);

    // Pull the three tokens off the first line
    std::istringstream line_stream(first_line);
    line_stream >> request.method >> request.path >> request.version;

    return request;
}