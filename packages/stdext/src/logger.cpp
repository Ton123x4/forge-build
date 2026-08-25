#include "stdext/logger.hpp"
#include "stdext/console.hpp"
#include <cstdio>
#include <vector>

namespace stdext {
    static std::vector<FILE*> internal_logger_sinks = { stdout };
    static FILE* internal_verbose_sink = nullptr;

    void logger::set_verbose_sink(FILE* sink) {
        internal_verbose_sink = sink;
    }

    FILE* logger::get_verbose_sink() {
        return internal_verbose_sink;
    }

    void logger::add_sink(FILE* sink) {
        if (!sink) {
            return;
        }

        internal_logger_sinks.push_back(sink);
    }

    void logger::remove_sink(FILE* sink) {
        std::erase(internal_logger_sinks, sink);
    }

    void logger::clear_sinks() {
        internal_logger_sinks.clear();
    }

    void logv(const std::string& message) {
        if (internal_verbose_sink) {
            std::fprintf(internal_verbose_sink, "%s", message.c_str());
        }
    }

    void logf(const std::string& message) {
        for (auto stream : internal_logger_sinks) {
            if (stream == stdout || stream == stderr) {
                std::fprintf(stream, "%s", message.c_str());
            } else {
                std::fprintf(stream, "%s", strip_ansi(message).c_str());
            }
        }
    }

    void logf(ansi_color color, const std::string& message) {
        set_fg_color(stdout, color);
        logf(message);
        reset_attr(stdout);
    }

    std::string log_body(const std::string& message, char end_char) {
        if (end_char == 0) {
            return message;
        }

        return std::format("{}{}", message, end_char);
    }

    std::string log_body(const std::string& title, const std::string& message, char end_char) {
        if (end_char == 0) {
            return std::format("{} {}", title, message);
        }

        return std::format("{} {}{}", title, message, end_char);
    }
}
