#include "stdext/version.hpp"

#include "stdext/filesystem.hpp"
#include "stdext/string.hpp"
#include "stdext/time.hpp"

#include <chrono>
#include <format>
#include <sstream>

#ifdef _WIN32
#define TIMEGM _mkgmtime
#else
#define TIMEGM timegm
#endif


namespace stdext {
    version_info load_version_file(const std::string& filepath) {
        auto file_content = stdext::fs::read_text(filepath);
        auto file_entries = stdext::string::split(file_content, " ");

        if (file_entries.size() < 2) {
            throw version_file_error(std::format("Invalid version file format: '{}'", filepath));
        }

        auto file_version = file_entries.at(0);
        auto file_timestamp = file_entries.at(1);
        auto version = stdext::version_info();

        {
            auto version_parts = stdext::string::split(file_version, ".");

            if (version_parts.size() != 3) {
                throw version_file_error(std::format("Invalid version format: '{}'", file_version));
            }

            try {
                version.first = std::stoul(version_parts.at(0));
                version.second = std::stoul(version_parts.at(1));
                version.build = std::stoul(version_parts.at(2));
            }
            catch (const std::exception& error) {
                throw version_file_error(std::format("Invalid version format: '{}'", file_version));
            }
        }

        {
            auto tm = std::tm{};
            auto stream = std::istringstream(file_timestamp);

            stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

            if (stream.fail()) {
                throw version_file_error(std::format("Invalid timestamp format: '{}'", file_timestamp));
            }

            version.timestamp = TIMEGM(&tm);
        }

        return version;
    }

    void store_version_file(const version_info& version, const std::string& filepath) {
        auto version_string = std::format("{}.{}.{}", version.first, version.second, version.build);

        if (version.timestamp == 0) {
            stdext::fs::write_file(filepath, std::format("{} {}", version_string, stdext::time::iso_seconds()));
        }
        else {
            stdext::fs::write_file(filepath, std::format("{} {}", version_string, time::unix_to_iso_seconds(version.timestamp)));
        }
    }
}
