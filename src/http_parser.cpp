#include "http_parser.h"
#include <sstream>
#include <algorithm>

HttpRequest HttpParser::parse(const std::string& raw) {
    HttpRequest request;
    std::istringstream stream(raw);

    // The first line of an HTTP request looks like:
    // GET /index.html HTTP/1.1
    // Everything after that is headers, which we ignore for now.
    std::string first_line;
    std::getline(stream, first_line);

    // Each HTTP line ends with \r\n and getline() uses \n as the delimiter
    // That means \r is still noise.
    if (!first_line.empty() && first_line.back() == '\r') {
        first_line.pop_back();
    }

    // Pull the three tokens off the first line.
    std::istringstream line_stream(first_line);
    line_stream >> request.method >> request.path >> request.version;

    std::string header_line;

    while (std::getline(stream, header_line)) {

        if (!header_line.empty() && (header_line.back() == '\r')) {
            header_line.pop_back();
        }

        if (header_line.empty()) break;

        size_t colon = header_line.find(':');
        // If no colon was found then continue.
        // int colon gives a warning and would rely on undefined behaviour.
        if (colon == std::string::npos) continue;

        std::string name = header_line.substr(0, colon);
        std::string value = header_line.substr(colon +1);

        size_t start = value.find_first_not_of(' ');

        if (start != std::string::npos) {
            value = value.substr(start);
        }

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        request.headers[name] = value;
    }

    return request;
}