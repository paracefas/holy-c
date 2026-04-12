#include "util.hpp"

#include <fstream>

std::expected<std::string, FileError> load_source(std::filesystem::path path) {
    if (!std::filesystem::exists(path)) 
        return std::unexpected(FileError::NotFound);

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) 
        return std::unexpected(FileError::ReadFailed);

    return std::string((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
}