#pragma once

#include <cstdint>
#include <string>

namespace stdext {
    enum class tok_kind;

    struct tok_location {
        std::string filepath;
        std::int32_t row = 1;
        std::int32_t column = 1;
        std::int32_t offset = 0;
    };

    struct token {
        tok_kind kind;
        tok_location loc;
        std::string value;
    };

    enum class tok_kind {
        unknown,

        binary,
        octal,
        hexadecimal,
        integer,
        floating,

        character,
        string,
        identifier,

        whitespace,
        new_line,

        at,
        dot,
        comma,
        caret,
        tilde,
        colon,
        semicolon,
        ampersand,
        currency,
        hashtag,

        plus,
        minus,
        asterisk,
        slash,
        pipe,
        percent,
        equal,
        question,
        exclamation,
        open_paren,
        close_paren,
        open_brace,
        close_brace,
        open_bracket,
        close_bracket,
        less_than,
        greater_than,
        less_than_equal,
        greater_than_equal,

        not_equal,
        arrow_left,
        arrow_right,

        double_colon,
        equal_equal,

        logical_and,
        logical_or,

        shift_left,
        shift_right,

        plus_equal,
        minus_equal,
        asterisk_equal,
        slash_equal,
        percent_equal,
        ampersand_equal,
        pipe_equal,
        caret_equal,
        shift_left_equal,
        shift_right_equal,

        end_of_source
    };
}
