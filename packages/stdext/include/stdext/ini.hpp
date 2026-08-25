#pragma once

#include "hash_map.hpp"
#include "stdext/scanner.hpp"

#include <string>

namespace stdext {
    using ini_section = stdext::hash_map<std::string, std::string>;
    using ini_sections = stdext::hash_map<std::string, ini_section>;

    struct ini_scanner_error : public stdext::scanner_error {
        explicit ini_scanner_error(const std::string& message)
            : stdext::scanner_error(message) {}

        explicit ini_scanner_error(const tok_location& location, const std::string& message)
            : stdext::scanner_error(location, message) {}
    };

    struct ini_syntax_error : public stdext::syntax_error {
        explicit ini_syntax_error(const stdext::token& token, const std::string& message)
            : stdext::syntax_error(token, message) {}
    };

    namespace ini {
        constexpr auto default_section = "root";

        ini_sections parse(const std::string& content);

        ini_sections parse_file(const std::string& filename);

        std::string dump(const ini_sections& sections);

        void dump_file(const ini_sections& sections, const std::string& filepath);
    }
}
