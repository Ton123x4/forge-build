#include "stdext/console.hpp"
#include <iostream>
#include <cstdio>
#include <regex>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define ISATTY_PLATFORM _isatty
#define FILENO_PLATFORM _fileno
#else
#define ISATTY_PLATFORM isatty
#define FILENO_PLATFORM fileno
#endif


void stdext::init_console() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    auto hIn  = GetStdHandle(STD_INPUT_HANDLE);

    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;

        if (GetConsoleMode(hOut, &mode)) {
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    if (hIn != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;

        if (GetConsoleMode(hIn, &mode)) {
            SetConsoleMode(hIn, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
    #else
    std::setlocale(LC_ALL, "");
    #endif
}

bool stdext::is_terminal(FILE* stream) {
    return ISATTY_PLATFORM(FILENO_PLATFORM(stream)) != 0;
}

void stdext::set_colors(FILE* stream, ansi_color fg_color, ansi_color bg_color) {
    std::fprintf(stream, "\033[%d;%dm", static_cast<int>(fg_color), static_cast<int>(bg_color) + 10);
}

void stdext::set_fg_color(FILE* stream, ansi_color fg_color) {
    std::fprintf(stream, "\033[%dm", static_cast<int>(fg_color));
}

void stdext::set_bg_color(FILE* stream, ansi_color bg_color) {
    std::fprintf(stream, "\033[%dm", static_cast<int>(bg_color) + 10);
}

void stdext::set_attr(FILE* stream, ansi_style style) {
    std::fprintf(stream, "\033[%dm", static_cast<int>(style));
}

void stdext::reset_attr(FILE* stream) {
    std::fprintf(stream, "\033[0m");
}


std::string stdext::text_attr(ansi_style style, const std::string& message) {
    return std::format("\033[{}m{}\033[0m", static_cast<int>(style), message);
}

std::string stdext::fg_color(ansi_color color, const std::string& message) {
    return std::format("\033[{}m{}\033[0m", static_cast<int>(color), message);
}

std::string stdext::bg_color(ansi_color color, const std::string& message) {
    return std::format("\033[{}m{}\033[0m", static_cast<int>(color) + 10, message);
}


std::string stdext::strip_ansi(const std::string& input) {
    auto pattern = std::regex("\\x1B\\[[0-9;?]*[a-zA-Z]");
    auto result = std::regex_replace(input, pattern, "");

    return result;
}


std::string stdext::input(const std::string& message) {
    auto buffer = std::string();

    std::printf("%s", message.c_str());
    std::getline(std::cin, buffer);

    return buffer;
}

void stdext::printc(stdext::ansi_color color, const std::string& message, char end_char) {
    fprintc(stdout, color, message, end_char);
}

void stdext::fprintc(FILE* stream, ansi_color color, const std::string& message, char end_char) {
    auto output = is_terminal(stream) ? message : strip_ansi(message);

    if (is_terminal(stream)) {
        stdext::set_fg_color(stdout, color);
    }

    std::fprintf(stream, "%s", output.c_str());

    if (end_char != '\0') {
        std::fputc(end_char, stream);
    }

    if (is_terminal(stream)) {
        stdext::reset_attr(stdout);
    }
}

void stdext::move_cursor(FILE* stream, ansi_move move, int32_t value) {
    switch (move) {
        case ansi_move::save_position:
        case ansi_move::restore_position:
            std::fprintf(stream, "\033[%c", static_cast<char>(move));
            break;

        default:
            std::fprintf(stream, "\033[%d%c", value, static_cast<char>(move));
            break;
    }
}

void stdext::set_cursor_position(FILE* stream, int32_t row, int32_t column) {
    std::fprintf(stream, "\033[%d;%dH", row, column);
}

void stdext::clear_line(FILE* stream, int32_t row) {
    move_cursor(stream, ansi_move::save_position);
    set_cursor_position(stream, row, 1);
    move_cursor(stream, ansi_move::clear_line, 2);
    move_cursor(stream, ansi_move::restore_position);
}

void stdext::clear_lines(FILE* stream, int32_t start_row, int32_t count) {
    move_cursor(stream, ansi_move::save_position);

    for (auto i = 0; i < count; i++) {
        set_cursor_position(stream, start_row + i, 1);
        move_cursor(stream, ansi_move::clear_line, 2);
    }

    move_cursor(stream, ansi_move::restore_position);
}

void stdext::clear_current_line(FILE* stream) {
    move_cursor(stdout, ansi_move::column);
    move_cursor(stdout, ansi_move::clear_line, 2);
}
