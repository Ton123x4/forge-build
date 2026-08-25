#include "stdext/ini.hpp"
#include "stdext/filesystem.hpp"
#include "stdext/parser.hpp"
#include "stdext/scanner.hpp"
#include "stdext/string.hpp"
#include "stdext/token.hpp"

#include <string>

namespace stdext {
    constexpr stdext::scanner_config kScannerConfig = {
        .line_comment = "#"
    };

    auto parse_entry_value(stdext::eager_parser& parser) {
        auto token = parser.advance();

        if (token.kind != stdext::tok_kind::identifier && token.kind != stdext::tok_kind::string) {
            throw ini_syntax_error(token, "expected an identifier or string as entry value");
        }

        return token.value;
    }

    void parse_section_entry(stdext::eager_parser& parser, ini_section& section, const stdext::token& key) {
        if (!parser.match(stdext::tok_kind::equal)) {
            throw ini_syntax_error(key, "expected '=' after entry key");
        }

        section.insert(key.value, parse_entry_value(parser));
    }

    auto parse_section_name(stdext::eager_parser& parser) {
        auto identifier = parser.advance();

        if (identifier.kind != stdext::tok_kind::identifier) {
            throw ini_syntax_error(identifier, "expected an identifier as section name");
        }

        if (!parser.match(tok_kind::close_bracket)) {
            throw ini_syntax_error(parser.peek(), "expected ']' after section name");
        }

        return identifier.value;
    }

    ini_sections parse_content(stdext::eager_parser& parser) {
        auto sections = ini_sections();

        auto section_name = std::string(ini::default_section);
        auto current_section = ini_section();

        while (!parser.is_end()) {
            auto token = parser.advance();

            switch (token.kind) {
                case stdext::tok_kind::identifier:
                    parse_section_entry(parser, current_section, token);
                    break;

                case stdext::tok_kind::open_bracket:
                    sections.insert(section_name, std::move(current_section));
                    section_name = parse_section_name(parser);
                    break;

                default:
                    throw ini_syntax_error(token, "unexpected token");
            }
        }

        sections.insert(section_name, current_section);

        return sections;
    }

    [[noreturn]] void raise_error(const stdext::scanner_error& error) {
        if (error.location) {
            throw ini_scanner_error(error.location.value(), error.what());
        }

        throw ini_scanner_error(error.what());
    }

    ini_sections ini::parse(const std::string& content) {
        try {
            auto scanner = stdext::scanner("root", content, kScannerConfig);
            auto parser = stdext::eager_parser(scanner);

            return parse_content(parser);
        }
        catch (const stdext::scanner_error& error) {
            raise_error(error);
        }
    }

    ini_sections ini::parse_file(const std::string& filename) {
        try {
            auto scanner = stdext::scanner(filename, kScannerConfig);
            auto parser = stdext::eager_parser(scanner);

            return parse_content(parser);
        }
        catch (const stdext::scanner_error& error) {
            raise_error(error);
        }
    }

    std::string ini::dump(const ini_sections& sections) {
        auto content = std::vector<std::string>();

        for (const auto& [name, section] : sections) {
            auto entries = std::vector<std::string>();

            for (const auto& [key, value] : section) {
                entries.push_back(key + " = " + value);
            }

            content.push_back("[" + name + "]");
            content.push_back(stdext::string::join(entries, "\n"));
        }

        return stdext::string::join(content, "\n\n");
    }

    void ini::dump_file(const ini_sections& sections, const std::string& filepath) {
        stdext::fs::write_file(filepath, dump(sections));
    }
}
