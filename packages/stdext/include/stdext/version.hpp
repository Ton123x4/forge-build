#pragma once

#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <string>

namespace stdext {
    struct version_info {
        std::int32_t first;
        std::int32_t second;
        std::int32_t build;
        std::time_t timestamp;
    };

    struct version_file_error : public std::runtime_error {
        explicit version_file_error(const std::string& message)
            : std::runtime_error(message) {
        }
    };

    version_info load_version_file(const std::string& filepath);

    void store_version_file(const version_info& version, const std::string& filepath);
}
