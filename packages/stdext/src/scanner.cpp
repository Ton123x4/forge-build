#include "stdext/scanner.hpp"

#include "stdext/filesystem.hpp"
#include "stdext/token.hpp"

#include <cctype>
#include <format>
#include <stdexcept>

namespace stdext {
    scanner::scanner(const std::string& filepath, scanner_config config) : m_current_loc(filepath), m_config(config) {
        try {
            m_source = fs::read_text(filepath);
            m_length = m_source.length();
        }
        catch (const std::runtime_error&) {
            throw scanner_error(std::format("failed to open input file: '{}'.", filepath));
        }
    }

    scanner::scanner(const std::string& name, const std::string& source, scanner_config config) : m_current_loc(name), m_source(source), m_config(config) {
        m_length = source.length();
    }

    bool scanner::is_eos() const {
        return is_end();
    }

    token scanner::next_token() {
        while (!is_end()) {
            auto start_loc = m_current_loc;
            auto character = next_char();

            if (!m_config.line_comment.empty()) {
                if (match_prefix(character, m_config.line_comment)) {
                    skip_line_comment();
                    continue;
                }
            }

            if (!m_config.comment_open.empty()) {
                if (match_prefix(character, m_config.comment_open)) {
                    skip_block_comment(start_loc);
                    continue;
                }
            }

            switch (character) {
                case ' ':
                case '\t':
                {
                    if (should_emit_whitespace(character)) {
                        return token(tok_kind::whitespace, start_loc, { character });
                    }

                    break;
                }

                case '\r':
                case '\n':
                {
                    if (should_emit_whitespace(character)) {
                        return token(tok_kind::new_line, start_loc, { character });
                    }

                    break;
                }

                case '@':
                    return token(tok_kind::at, start_loc, { character });

                case '.':
                    return token(tok_kind::dot, start_loc, { character });

                case ',':
                    return token(tok_kind::comma, start_loc, { character });

                case '~':
                    return token(tok_kind::tilde, start_loc, { character });

                case ';':
                    return token(tok_kind::semicolon, start_loc, { character });

                case '$':
                    return token(tok_kind::currency, start_loc, { character });

                case '?':
                    return token(tok_kind::question, start_loc, { character });

                case '(':
                    return token(tok_kind::open_paren, start_loc, { character });

                case ')':
                    return token(tok_kind::close_paren, start_loc, { character });

                case '{':
                    return token(tok_kind::open_brace, start_loc, { character });

                case '}':
                    return token(tok_kind::close_brace, start_loc, { character });

                case '[':
                    return token(tok_kind::open_bracket, start_loc, { character });

                case ']':
                    return token(tok_kind::close_bracket, start_loc, { character });

                case '"':
                case '\'':
                    return scan_string(start_loc, character);

                case '#':
                {
                    if (std::isxdigit(peek_char())) {
                        return scan_hexadecimaal(start_loc, character);
                    }

                    return token(tok_kind::hashtag, start_loc, { character });
                }

                case ':':
                {
                    if (!is_end() && consume_char(':')) {
                        return token(tok_kind::double_colon, start_loc, "::");
                    }

                    return token(tok_kind::colon, start_loc, { character });
                }

                case '=':
                {
                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::equal_equal, start_loc, "==");
                    }

                    return token(tok_kind::equal, start_loc, { character });
                }

                case '!':
                {
                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::not_equal, start_loc, "!=");
                    }

                    return token(tok_kind::exclamation, start_loc, { character });
                }

                case '+':
                {
                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::plus_equal, start_loc, "+=");
                    }

                    return token(tok_kind::plus, start_loc, { character });
                }

                case '-':
                {
                    if (!is_end() && consume_char('>')) {
                        return token(tok_kind::arrow_right, start_loc, "->");
                    }

                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::minus_equal, start_loc, "-=");
                    }

                    return token(tok_kind::minus, start_loc, { character });
                }

                case '*':
                {
                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::asterisk_equal, start_loc, "*=");
                    }

                    return token(tok_kind::asterisk, start_loc, { character });
                }

                case '/':
                {
                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::slash_equal, start_loc, "/=");
                    }

                    return token(tok_kind::slash, start_loc, { character });
                }

                case '%':
                {
                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::percent_equal, start_loc, "%=");
                    }

                    return token(tok_kind::percent, start_loc, { character });
                }

                case '&':
                {
                    if (!is_end()) {
                        if (consume_char('&')) {
                            return token(tok_kind::logical_and, start_loc, "&&");
                        }

                        if (consume_char('=')) {
                            return token(tok_kind::ampersand_equal, start_loc, "&=");
                        }
                    }

                    return token(tok_kind::ampersand, start_loc, { character });
                }

                case '|':
                {
                    if (!is_end()) {
                        if (consume_char('|')) {
                            return token(tok_kind::logical_or, start_loc, "||");
                        }

                        if (consume_char('=')) {
                            return token(tok_kind::pipe_equal, start_loc, "|=");
                        }
                    }

                    return token(tok_kind::pipe, start_loc, { character });
                }

                case '^':
                {
                    if (!is_end() && consume_char('=')) {
                        return token(tok_kind::caret_equal, start_loc, "^=");
                    }

                    return token(tok_kind::caret, start_loc, { character });
                }

                case '<':
                {
                    if (!is_end()) {
                        if (consume_char('=')) {
                            return token(tok_kind::less_than_equal, start_loc, "<=");
                        }

                        if (consume_char('<')) {
                            if (!is_end() && consume_char('=')) {
                                return token(tok_kind::shift_left_equal, start_loc, "<<=");
                            }

                            return token(tok_kind::shift_left, start_loc, "<<");
                        }

                        if (consume_char('-')) {
                            return token(tok_kind::arrow_left, start_loc, "<-");
                        }
                    }

                    return token(tok_kind::less_than, start_loc, { character });
                }

                case '>':
                {
                    if (!is_end()) {
                        if (consume_char('=')) {
                            return token(tok_kind::greater_than_equal, start_loc, ">=");
                        }

                        if (consume_char('>')) {
                            if (!is_end() && consume_char('=')) {
                                return token(tok_kind::shift_right_equal, start_loc, ">>=");
                            }

                            return token(tok_kind::shift_right, start_loc, ">>");
                        }
                    }

                    return token(tok_kind::greater_than, start_loc, { character });
                }

                default:
                {
                    if (std::isdigit(character)) {
                        return scan_number(start_loc, character);
                    }

                    if (character == '_' || std::isalpha(character)) {
                        return scan_identifier(start_loc, character);
                    }

                    return token(tok_kind::unknown, start_loc, { character });
                }
            }
        }

        return token(tok_kind::end_of_source, m_current_loc, { '\255' });
    }

    bool scanner::should_emit_whitespace(char ch) const {
        switch (ch) {
            case ' ':
                return m_config.allow_whitespace;

            case '\t':
                return m_config.allow_tab_space;

            case '\n':
            case '\r':
                return m_config.allow_new_line;
        }

        return false;
    }

    bool scanner::match_prefix(char first, const std::string& prefix) {
        if (prefix.empty() || first != prefix[0]) {
            return false;
        }

        for (std::size_t i = 1; i < prefix.size(); ++i) {
            if (look_ahead(static_cast<int>(i - 1)) != prefix[i]) {
                return false;
            }
        }

        for (std::size_t i = 1; i < prefix.size(); ++i) {
            next_char();
        }

        return true;
    }

    void scanner::skip_block_comment(tok_location start_loc) {
        auto remaining_blocks = 1;

        while (!is_end() && remaining_blocks > 0) {
            char character = next_char();

            if (match_prefix(character, m_config.comment_open)) {
                remaining_blocks++;
            }
            else if (match_prefix(character, m_config.comment_close)) {
                remaining_blocks--;
            }
        }

        if (remaining_blocks != 0) {
            throw scanner_error(start_loc, "unclosed block comment.");
        }
    }

    void scanner::skip_line_comment() {
        while (!is_end() && peek_char() != '\n') {
            next_char();
        }
    }

    template <typename check_fn>
    std::string scanner::read_number(char first_char, check_fn is_valid) {
        auto value = std::string();

        if (first_char != 0) {
            value.push_back(first_char);
        }

        while (!is_end()) {
            char character = peek_char();

            if (character == '_') {
                next_char();
                continue;
            }

            if (!is_valid(character)) {
                break;
            }

            value.push_back(next_char());
        }

        return value;
    }

    std::string scanner::read_number(char first_char) {
        return read_number(first_char, [](char character) {
            return std::isdigit(character);
        });
    }

    token scanner::scan_hexadecimaal(tok_location start_loc, char character) {
        auto value = read_number(0, [](char character) {
            return std::isxdigit(character);
        });

        if (value.empty()) {
            throw scanner_error(start_loc, "expected digits after hexadecimal prefix '0x'.");
        }

        return stdext::token(tok_kind::hexadecimal, start_loc, value);
    }

    token scanner::scan_number(tok_location start_loc, char first_char) {
        if (first_char == '0') {
            if (consume_char('x')) {
                return scan_hexadecimaal(start_loc, 0);
            }

            if (consume_char('b')) {
                auto value = read_number(0, [](char character) {
                    return character == '0' || character == '1';
                });

                if (value.empty()) {
                    throw scanner_error(start_loc, "expected digits after binary prefix '0b'.");
                }

                return stdext::token(tok_kind::binary, start_loc, value);
            }

            if (consume_char('o')) {
                auto value = read_number(0, [](char character) {
                    return character >= '0' && character <= '7';
                });

                if (value.empty()) {
                    throw scanner_error(start_loc, "expected digits after octal prefix '0c'.");
                }

                return stdext::token(tok_kind::octal, start_loc, value);
            }
        }

        return scan_integer(start_loc, first_char);
    }

    token scanner::scan_integer(tok_location start_loc, char first_char) {
        auto value = read_number(first_char);

        if (!is_end()) {
            if (consume_char('e')) {
                value.push_back(current_char());
                return scan_expoent(start_loc, value);
            }

            if (consume_char('.')) {
                value.push_back(current_char());

                if (!std::isdigit(peek_char())) {
                    throw scanner_error(start_loc, "expected digits after decimal.");
                }

                return scan_float(start_loc, value);
            }
        }

        return token(tok_kind::integer, start_loc, value);
    }

    token scanner::scan_float(tok_location start_loc, std::string value) {
        value.append(read_number());

        if (!is_end() && check_char('e')) {
            value.push_back(next_char());
            return scan_expoent(start_loc, value);
        }

        return token(tok_kind::floating, start_loc, value);
    }

    token scanner::scan_expoent(tok_location start_loc, std::string value) {
        if (check_char('+') || check_char('-')) {
            value.push_back(next_char());
        }

        if (is_end() || !std::isdigit(peek_char())) {
            throw scanner_error(start_loc, "expected digits after exponent.");
        }

        value.append(read_number());

        return token(tok_kind::floating, start_loc, value);
    }

    char scanner::resolve_escape(char character) {
        // Source: https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_special_characters?view=powershell-7.5

        switch (character) {
            case '0':
                return '\0';

            case 'a':
                return '\a';

            case 'b':
                return '\b';

            case 'e':
                return '\e';

            case 'f':
                return '\f';

            case 'n':
                return '\n';

            case 'r':
                return '\r';

            case 't':
                return '\t';

            case 'v':
                return '\v';

            default:
                return character;
        }
    }

    token scanner::scan_string(tok_location start_loc, char end_char) {
        auto value = std::string();
        auto closed = false;

        while (!is_end()) {
            char character = next_char();

            if (character == end_char) {
                closed = true;
                break;
            }

            if (character == '\\') {
                if (is_end()) {
                    break;
                }

                character = next_char();
                character = resolve_escape(character);
            }

            value.push_back(character);
        }

        if (closed == false) {
            throw scanner_error(start_loc, "unexpected end of file.");
        }

        auto value_kind = tok_kind::character;

        if (end_char == '"') {
            value_kind = tok_kind::string;
        }

        return token(value_kind, start_loc, value);
    }

    token scanner::scan_identifier(tok_location start_loc, char first_char) {
        auto identifier = std::string();

        if (first_char) {
            identifier.push_back(first_char);
        }

        while (!is_end()) {
            char character = peek_char();

            if (character != '_' && !std::isalnum(character)) {
                break;
            }

            identifier.push_back(character);
            next_char();
        }

        return token(tok_kind::identifier, start_loc, identifier);
    }

    bool scanner::is_end() const {
        return m_current_loc.offset == m_length;
    }

    char scanner::next_char() {
        char character = m_source[m_current_loc.offset++];

        if (character == '\n') {
            m_current_loc.row++;
            m_current_loc.column = 1;
        }
        else if (character != '\r') {
            m_current_loc.column++;
        }

        return character;
    }

    char scanner::peek_char() const {
        if (m_current_loc.offset >= m_length) {
            return '\0';
        }

        return m_source[m_current_loc.offset];
    }

    char stdext::scanner::look_ahead(int offset) const {
        auto index = m_current_loc.offset + offset;

        if (index >= m_length) {
            return '\0';
        }

        return m_source[index];
    }

    char scanner::current_char() const {
        return m_source[m_current_loc.offset - 1];
    }

    bool scanner::check_char(char ch) const {
        char character = peek_char();
        auto result = character == ch;

        return result;
    }

    bool scanner::consume_char(char ch) {
        auto result = check_char(ch);

        if (result) {
            next_char();
        }

        return result;
    }
}
