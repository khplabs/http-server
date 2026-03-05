#include "file_handler.h"
#include <fstream>
#include <sstream>

FileHandler::FileHandler(const std::string& base_path): base_path(base_path) {}

FileResult FileHandler::read(const std::string& path) {
    FileResult result;
    result.found = false;

    std::string file_path = path;
    if (file_path == "/") {
        file_path = "/index.html";
    }

    std::string full_path = base_path + file_path;

    std::ifstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
        return result;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    result.content = buffer.str();
    result.found = true;
    result.mime_type = resolve_mime_type(file_path);

    return result;
}

std::string FileHandler::resolve_mime_type(const std::string& path) {

    size_t file_extension_dot = path.rfind('.');
    if (file_extension_dot == std::string::npos) return "application/octet-stream";

    std::string file_extension = path.substr(file_extension_dot);

    if (file_extension == ".html") return "text/html";
    if (file_extension == ".css") return "text/css";
    if (file_extension == ".js") return "application/javascript";
    if (file_extension == ".json") return "application/json";
    if (file_extension == ".png") return "image/png";
    if (file_extension == ".jpg") return "image/jpeg";
    if (file_extension == ".ico") return "image/x-icon";

    return "application/octet-stream";
}