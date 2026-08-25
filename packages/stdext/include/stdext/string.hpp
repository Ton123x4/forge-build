#pragma once

#include <charconv>
#include <optional>
#include <vector>
#include <string>

namespace stdext::string {
    std::string trim_left(std::string_view string);
    std::string trim_right(std::string_view string);
    std::string trim(std::string_view string);

    bool starts_with(std::string_view string, std::string_view prefix);
    bool ends_with(std::string_view string, std::string_view suffix);
    bool contains(std::string_view string, std::string_view substr);
    bool is_empty(std::string_view string);

    std::vector<std::string> split(const std::string& string, char delimiter);
    std::vector<std::string> split(const std::string& string, std::string_view delimiter);

    std::string wrap(const std::vector<std::string>& parts, std::string_view item);
    std::string join(const std::vector<std::string>& parts, std::string_view delimiter);
    std::string replace(std::string_view string, std::string_view from, std::string_view to);
    std::string replace_all(std::string_view string, std::string_view from, std::string_view to);

    std::string to_upper(std::string_view string);
    std::string to_lower(std::string_view string);
    std::string to_snake_case(std::string_view string);
    std::string to_camel_case(const std::string& string);
    std::string to_pascal_case(const std::string& string);

    std::string repeat(std::string_view string, std::size_t n);
    std::string pad_left(const std::string& string, std::size_t width, char fill = ' ');
    std::string pad_right(const std::string& string, std::size_t width, char fill = ' ');
    std::string center(const std::string& string, std::size_t width, char fill = ' ');
    std::size_t count(std::string_view string, std::string_view substr);
    std::string reverse(std::string_view string);

    template<typename T>
    std::optional<T> to(std::string_view string) {
        auto result = T{};
        auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), result);

        if (ec == std::errc{} && ptr == string.data() + string.size()) {
            return result;
        }

        return std::nullopt;
    }
}
