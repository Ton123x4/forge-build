#pragma once

#include <optional>
#include <stdexcept>

#include "token.hpp"

namespace stdext {
    struct scanner_error : public std::runtime_error {
        std::optional<tok_location> location;

        scanner_error(const std::string& message)
            : std::runtime_error(message), location(std::nullopt) {
        }

        scanner_error(const tok_location& location, const std::string& message)
            : std::runtime_error(message), location(location) {
        }
    };

    struct syntax_error : public std::runtime_error {
        stdext::token token;

        syntax_error(const stdext::token& token, const std::string& message)
            : std::runtime_error(message), token(token) {
        }
    };

    struct scanner_config {
        bool allow_whitespace = false;
        bool allow_tab_space = false;
        bool allow_new_line = false;
        std::string line_comment;
        std::string comment_open;
        std::string comment_close;
    };

    class scanner {
    public:
        scanner(const std::string& filepath, scanner_config config = {});
        scanner(const std::string& name, const std::string& source, scanner_config config = {});

        token next_token();
        bool is_eos() const;

    private:
        scanner_config m_config;
        tok_location m_current_loc;
        std::string m_source;
        std::int32_t m_length;

        bool should_emit_whitespace(char ch) const;

        bool match_prefix(char first, const std::string& prefix);
        void skip_block_comment(tok_location start_loc);
        void skip_line_comment();

        template <typename check_fn>
        std::string read_number(char first_char, check_fn is_valid);
        std::string read_number(char first_char = 0);

        token scan_hexadecimaal(tok_location start_loc, char character);
        token scan_integer(tok_location start_loc, char character);
        token scan_number(tok_location start_loc, char character);
        token scan_float(tok_location start_loc, std::string value);
        token scan_expoent(tok_location start_loc, std::string value);

        char resolve_escape(char character);
        token scan_string(tok_location start_loc, char character);
        token scan_identifier(tok_location start_loc, char character);

        bool is_end() const;
        char next_char();
        char peek_char() const;
        char current_char() const;
        char look_ahead(int offset) const;

        bool check_char(char ch) const;
        bool consume_char(char ch);
    };
}
