#pragma once
#include <string>

struct FileResult {
    bool found;
    std::string content;
    std::string mime_type;
};

class FileHandler {
    public:
        FileHandler(const std::string& base_path);
        FileResult read(const std::string& path);

    private:
        std::string base_path;
        std::string resolve_mime_type(const std::string& path);
};