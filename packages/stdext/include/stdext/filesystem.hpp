#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace stdext::fs {
    namespace std_fs = std::filesystem;

    using std_fs::path;
    using std_fs::directory_entry;
    using std_fs::directory_iterator;
    using std_fs::recursive_directory_iterator;
    using std_fs::file_status;
    using std_fs::file_time_type;
    using std_fs::space_info;
    using std_fs::perms;
    using std_fs::perm_options;
    using std_fs::copy_options;
    using std_fs::directory_options;
    using std_fs::filesystem_error;
    using std_fs::file_type;

    using namespace std_fs;

    std::string read_text(const fs::path& path);
    std::vector<char> read_file(const fs::path& path);
    std::vector<std::string> read_lines(const fs::path& path);

    std::uint64_t hash_file(const fs::path& path);
    std::uint64_t hash_files(std::vector<std::string> files);

    void write_file(const fs::path& path, const std::string& content);
    void append_file(const fs::path& path, const std::string& content);

    size_t file_size(const fs::path& path);

    void move(const fs::path& from, const fs::path& to);

    inline std::string parent_path(const fs::path& path) {
        return path.parent_path().string();
    }

    inline std::string filename(const fs::path& path) {
        return path.filename().string();
    }

    inline std::string extension(const fs::path& path) {
        return path.extension().string();
    }

    inline std::string stem(const fs::path& path) {
        return path.stem().string();
    }

    inline std::string current_dir() {
        return std_fs::current_path().string();
    }

    inline std::string temporary_dir() {
        return std_fs::temp_directory_path().string();
    }

    inline std::string canonical(const fs::path& path) {
        return std_fs::weakly_canonical(path).string();
    }

    inline std::string absolute(const fs::path& path) {
        return std_fs::absolute(path).string();
    }

    inline std::string relative(const fs::path& path, const fs::path& base = std_fs::current_path()) {
        return std_fs::relative(path, base).string();
    }

    inline std::string sanitize(const fs::path& path) {
        return path.lexically_normal().string();
    }

    inline std::string sanitize_posix(const fs::path& path) {
        return path.lexically_normal().generic_string();
    }

    inline std::string root_name(const fs::path& path) {
        return path.root_name().string();
    }

    inline std::string root_path(const fs::path& path) {
        return path.root_path().string();
    }

    inline std::string root_directory(const fs::path& path) {
        return path.root_directory().string();
    }

    inline std::string replace_extension(path path, const std::string& extension) {
        return path.replace_extension(extension).string();
    }

    inline bool has_stem(const fs::path& path) {
        return path.has_stem();
    }

    inline bool has_filename(const fs::path& path) {
        return path.has_filename();
    }

    inline bool has_extension(const fs::path& path) {
        return path.has_extension();
    }

    inline bool has_parent_path(const fs::path& path) {
        return path.has_parent_path();
    }
    inline bool has_relative_path(const fs::path& path) {
        return path.has_relative_path();
    }

    inline bool has_root_name(const fs::path& path) {
        return path.has_root_name();
    }

    inline bool has_root_path(const fs::path& path) {
        return path.has_root_path();
    }

    inline bool has_root_directory(const fs::path& path) {
        return path.has_root_directory();
    }

    inline bool is_relative(const fs::path& path) {
        return path.is_relative();
    }

    inline bool is_absolute(const fs::path& path) {
        return path.is_absolute();
    }

    template <typename... Args>
    inline std::string join_path(const fs::path& first, Args&&... args) {
        path result = first;

        (result /= ... /= std::forward<Args>(args));

        return sanitize(result.string());
    }
}
