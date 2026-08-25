#include "core/dependency.h"

#include "stdext/filesystem.hpp"
#include "stdext/string.hpp"

namespace DependencyFile {
    static std::string joinContinuedLines(const std::string& content) {
        return stdext::string::replace_all(content, "\\\n", " ");
    }

    static std::vector<std::string> splitEscapedWords(const std::string& line) {
        auto raw_words = stdext::string::split(line, ' ');
        auto words = std::vector<std::string>();

        for (const auto& raw_word : raw_words) {
            auto trimmed = stdext::string::trim(raw_word);

            if (stdext::string::is_empty(trimmed)) {
                continue;
            }

            if (!words.empty() && stdext::string::ends_with(words.back(), "\\")) {
                words.back().back() = ' ';
                words.back() += trimmed;
                continue;
            }

            words.push_back(trimmed);
        }

        return words;
    }

    std::vector<std::string> ParseDependencies(const std::string& filepath) {
        auto content = stdext::fs::read_text(filepath);

        auto joined = joinContinuedLines(content);
        auto colon_pos = joined.find(':');

        if (colon_pos == std::string::npos) {
            return {};
        }

        return splitEscapedWords(joined.substr(colon_pos + 1));
    }
}
