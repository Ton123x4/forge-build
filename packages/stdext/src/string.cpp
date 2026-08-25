#include "stdext/string.hpp"
#include <algorithm>
#include <format>
#include <sstream>

std::string stdext::string::trim_left(std::string_view string) {
    auto start = std::find_if(string.begin(), string.end(), [](unsigned char c) {
        return !std::isspace(c);
    });

    return std::string(start, string.end());
}

std::string stdext::string::trim_right(std::string_view string) {
    auto end = std::find_if(string.rbegin(), string.rend(), [](unsigned char c) {
        return !std::isspace(c);
    });

    return std::string(string.begin(), end.base());
}

std::string stdext::string::trim(std::string_view string) {
    return trim_left(trim_right(string));
}

bool stdext::string::starts_with(std::string_view string, std::string_view prefix) {
    return string.starts_with(prefix);
}

bool stdext::string::ends_with(std::string_view string, std::string_view suffix) {
    return string.ends_with(suffix);
}

bool stdext::string::contains(std::string_view string, std::string_view substr) {
    return string.find(substr) != std::string_view::npos;
}

bool stdext::string::is_empty(std::string_view string) {
    return trim(string).empty();
}

std::vector<std::string> stdext::string::split(const std::string& string, char delimiter) {
    auto stream = std::istringstream(string);
    auto parts = std::vector<std::string>();
    auto token = std::string();

    while (std::getline(stream, token, delimiter)) {
        parts.push_back(token);
    }

    return parts;
}

std::vector<std::string> stdext::string::split(const std::string& string, std::string_view delimiter) {
    auto parts = std::vector<std::string>();
    auto position = std::size_t(0);

    while (true) {
        auto found = string.find(delimiter, position);

        if (found == std::string::npos) {
            parts.push_back(string.substr(position));
            break;
        }

        parts.push_back(string.substr(position, found - position));
        position = found + delimiter.size();
    }

    return parts;
}

std::string stdext::string::wrap(const std::vector<std::string>& parts, std::string_view item) {
    std::string result;

    for (auto& part : parts) {
        if (!result.empty()) {
            result.push_back(' ');
        }

        result += std::format("{}{}{}", item, part, item);
    }

    return result;
}

std::string stdext::string::join(const std::vector<std::string>& parts, std::string_view delimiter) {
    std::string result;

    for (auto& part : parts) {
        if (!result.empty()) {
            result += delimiter;
        }

        result += part;
    }

    return result;
}

std::string stdext::string::replace(std::string_view string, std::string_view from, std::string_view to) {
    auto result = std::string(string);
    auto position = result.find(from);

    if (position != std::string::npos) {
        result.replace(position, from.size(), to);
    }

    return result;
}

std::string stdext::string::replace_all(std::string_view string, std::string_view from, std::string_view to) {
    auto result = std::string(string);
    auto position = std::size_t(0);

    while (true) {
        position = result.find(from, position);

        if (position == std::string::npos) {
            break;
        }

        result.replace(position, from.size(), to);
        position += to.size();
    }

    return result;
}

std::string stdext::string::to_upper(std::string_view string) {
    auto result = std::string(string);

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::toupper(c);
    });

    return result;
}

std::string stdext::string::to_lower(std::string_view string) {
    auto result = std::string(string);

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    return result;
}

std::string stdext::string::to_snake_case(std::string_view string) {
    auto result = std::string();

    for (auto i = std::size_t(0); i < string.size(); ++i) {
        auto c = string[i];

        if (std::isupper(c) && i > 0) {
            result += '_';
        }

        result += std::tolower(c);
    }

    return result;
}

std::string stdext::string::to_pascal_case(const std::string& string) {
    auto result = std::string();
    auto parts = split(string, '_');

    for (auto& part : parts) {
        if (part.empty()) {
            continue;
        }

        result += std::toupper(part[0]);
        result += part.substr(1);
    }

    return result;
}

std::string stdext::string::to_camel_case(const std::string& string) {
    auto result = to_pascal_case(string);

    if (!result.empty()) {
        result[0] = std::tolower(result[0]);
    }

    return result;
}


std::string stdext::string::repeat(std::string_view string, std::size_t n) {
    auto result = std::string();

    result.reserve(string.size() * n);

    for (auto i = std::size_t(0); i < n; ++i) {
        result += string;
    }

    return result;
}

std::string stdext::string::pad_left(const std::string& string, std::size_t width, char fill) {
    if (string.size() >= width) {
        return string;
    }

    return repeat(std::string(1, fill), width - string.size()) + string;
}

std::string stdext::string::pad_right(const std::string& string, std::size_t width, char fill) {
    if (string.size() >= width) {
        return string;
    }

    return string + repeat(std::string(1, fill), width - string.size());
}

std::string stdext::string::center(const std::string& string, std::size_t width, char fill) {
    if (string.size() >= width) {
        return string;
    }

    auto total = width - string.size();
    auto left = total / 2;
    auto right = total - left;

    return repeat(std::string(1, fill), left) + string + repeat(std::string(1, fill), right);
}

std::size_t stdext::string::count(std::string_view string, std::string_view substr) {
    auto result = std::size_t(0);
    auto position = std::size_t(0);

    while (true) {
        position = string.find(substr, position);

        if (position == std::string_view::npos) {
            break;
        }

        position += substr.size();
        ++result;
    }

    return result;
}

std::string stdext::string::reverse(std::string_view string) {
    return std::string(string.rbegin(), string.rend());
}

template<>
std::optional<std::string> stdext::string::to<std::string>(std::string_view string) {
    return std::string(string);
}

template<>
std::optional<bool> stdext::string::to<bool>(std::string_view string) {
    auto lower = to_lower(string);

    if (lower == "true" || lower == "1" || lower == "sim" || lower == "s" || lower == "yes" || lower == "y") {
        return true;
    }

    if (lower == "false" || lower == "0" || lower == "não" || lower == "nao" || lower == "no" || lower == "n") {
        return false;
    }

    return std::nullopt;
}
