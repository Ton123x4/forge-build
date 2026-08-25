#include "stdext/filesystem.hpp"
#include "stdext/crypto.hpp"
#include "stdext/string.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <format>

namespace stdext {
    std::vector<char> fs::read_file(const fs::path& path) {
        auto stream = std::ifstream(path, std::ios::binary);

        if (!stream.is_open()) {
            throw std::runtime_error(std::format("Cannot open file '{}'", path.string()));
        }

        auto size = fs::file_size(path);
        auto buffer = std::vector<char>(size);

        stream.read(buffer.data(), size);
        stream.close();

        return buffer;
    }

    std::string fs::read_text(const fs::path& path) {
        auto stream = std::ifstream(path);

        if (!stream.is_open()) {
            throw std::runtime_error(std::format("Cannot open text file '{}'", path.string()));
        }

        auto buffer = std::ostringstream();

        buffer << stream.rdbuf();

        return buffer.str();
    }

    std::vector<std::string> fs::read_lines(const fs::path& path) {
        auto text_content = read_text(path);
        auto text_lines = string::split(text_content, "\n");

        return text_lines;
    }

    std::uint64_t fs::hash_file(const fs::path& path) {
        auto file = std::ifstream(path, std::ios::binary);
        auto ctx = crypto::hash::fnv_context();
        auto buffer = std::array<char, 8192>();

        while (file.read(buffer.data(), buffer.size())) {
            ctx.update(buffer.data(), file.gcount());
        }

        if (file.gcount() > 0) {
            ctx.update(buffer.data(), file.gcount());
        }

        return ctx.finalize();
    }

    std::uint64_t fs::hash_files(std::vector<std::string> files) {
        auto ctx = stdext::crypto::hash::fnv_context();

        std::sort(files.begin(), files.end());

        for (const auto& path : files) {
            if (!stdext::fs::is_regular_file(path)) {
                continue;
            }

            auto file = std::ifstream(path, std::ios::binary);
            auto buffer = std::array<char, 8192>();

            while (file.read(buffer.data(), buffer.size())) {
                ctx.update(buffer.data(), file.gcount());
            }

            if (file.gcount() > 0) {
                ctx.update(buffer.data(), file.gcount());
            }
        }

        return ctx.finalize();
    }

    void fs::write_file(const fs::path& path, const std::string& content) {
        auto parent = stdext::fs::parent_path(path);

        if (!parent.empty() && !std_fs::exists(parent)) {
            fs::create_directories(parent);
        }

        auto stream = std::ofstream(path, std::ios::binary);

        if (!stream.is_open()) {
            throw std::runtime_error(std::format("Cannot write file '{}'", path.string()));
        }

        stream << content;
    }

    void fs::append_file(const fs::path& path, const std::string& content) {
        auto parent = stdext::fs::parent_path(path);

        if (!parent.empty() && !fs::exists(parent)) {
            fs::create_directories(parent);
        }

        auto stream = std::ofstream(path, std::ios::binary | std::ios::app);

        if (!stream.is_open()) {
            throw std::runtime_error(std::format("Cannot open file '{}' for appending", path.string()));
        }

        stream << content;
    }

    std::size_t fs::file_size(const fs::path& path) {
        auto error = std::error_code();
        auto size = std_fs::file_size(path, error);

        if (error) {
            return 0;
        }

        return size;
    }

    void fs::move(const fs::path& from, const fs::path& to) {
        auto error = std::error_code();

        std_fs::rename(from, to, error);

        if (error) {
            if (error == std::errc::cross_device_link) {
                std_fs::copy(from, to, std_fs::copy_options::recursive, error);

                if (error) {
                    throw std::runtime_error(std::format("Cannot move '{}' to '{}': {}", from.string(), to.string(), error.message()));
                }

                std_fs::remove_all(from, error);

                if (error) {
                    throw std::runtime_error(std::format("Cannot remove '{}' after move: {}", from.string(), error.message()));
                }
            } else {
                throw std::runtime_error(std::format("Cannot move '{}' to '{}': {}", from.string(), to.string(), error.message()));
            }
        }
    }
}
