#pragma once

#include <format>
#include <string>

namespace stdext {
    enum class ansi_style : int {
        reset = 0,
        bold = 1,
        dim = 2,
        italic = 3,
        underline = 4,
        blink = 5,
        inverse = 7,
        hidden = 8,
        strikethrough = 9,
    };

    enum class ansi_color : int {
        black = 30,
        red = 31,
        green = 32,
        yellow = 33,
        blue = 34,
        magenta = 35,
        cyan = 36,
        white = 37,
        default_color = 39,

        bright_black = 90,
        bright_red = 91,
        bright_green = 92,
        bright_yellow = 93,
        bright_blue = 94,
        bright_magenta = 95,
        bright_cyan = 96,
        bright_white = 97,
    };

    enum class ansi_move : char {
        up = 'A',
        down = 'B',
        right = 'C',
        left = 'D',
        next_line = 'E',
        previous_line = 'F',
        column = 'G',
        position = 'H',
        clear_screen = 'J',
        clear_line = 'K',
        scroll_up = 'S',
        scroll_down = 'T',
        save_position = 's',
        restore_position = 'u'
    };

    void init_console();

    bool is_terminal(FILE* stream);

    void set_colors(FILE* stream, ansi_color fg_color, ansi_color bg_color);

    void set_fg_color(FILE* stream, ansi_color fg_color);

    void set_bg_color(FILE* stream, ansi_color bg_color);

    void set_attr(FILE* stream, ansi_style style);

    void reset_attr(FILE* stream);

    std::string text_attr(ansi_style color, const std::string& message);

    std::string fg_color(ansi_color color, const std::string& message);

    std::string bg_color(ansi_color color, const std::string& message);

    std::string strip_ansi(const std::string& input);

    std::string input(const std::string& message = "");

    void printc(ansi_color color, const std::string& message, char end_char);

    void fprintc(FILE* stream, ansi_color color, const std::string& message, char end_char);

    void move_cursor(FILE* stream, ansi_move move, int value = 1);

    void set_cursor_position(FILE* stream, int row, int column);

    void clear_line(FILE* stream, int row);

    void clear_lines(FILE* stream, int start_row, int count);

    void clear_current_line(FILE* stream);

    template <typename... Args>
    void print(std::format_string<Args...> fmt, Args&&... args) {
        printc(ansi_color::default_color, std::format(fmt, std::forward<Args>(args)...), 0);
    }

    template <typename... Args>
    void println(std::format_string<Args...> fmt = "", Args&&... args) {
        printc(ansi_color::default_color, std::format(fmt, std::forward<Args>(args)...), '\n');
    }

    template <typename... Args>
    void print(ansi_color color, std::format_string<Args...> fmt, Args&&... args) {
        printc(color, std::format(fmt, std::forward<Args>(args)...), 0);
    }

    template <typename... Args>
    void println(ansi_color color, std::format_string<Args...> fmt, Args&&... args) {
        printc(color, std::format(fmt, std::forward<Args>(args)...), '\n');
    }
}
