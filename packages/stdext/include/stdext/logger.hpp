#pragma once

#include "stdext/console.hpp"
#include "stdext/string.hpp"

#ifndef STDEXT_LOG_TITLE
#define STDEXT_LOG_TITLE "(>)"
#endif

#ifndef STDEXT_PROMPT_TITLE
#define STDEXT_PROMPT_TITLE "(<)"
#endif

#ifndef STDEXT_CONFIRM_TITLE
#define STDEXT_CONFIRM_TITLE "(<)"
#endif

#ifndef STDEXT_INFO_TITLE
#define STDEXT_INFO_TITLE "(!)"
#endif

#ifndef STDEXT_UPDATE_TITLE
#define STDEXT_UPDATE_TITLE "(~)"
#endif

#ifndef STDEXT_VERBOSE_TITLE
#define STDEXT_VERBOSE_TITLE "(+)"
#endif

#ifndef STDEXT_WARN_TITLE
#define STDEXT_WARN_TITLE "(!)"
#endif

#ifndef STDEXT_ERROR_TITLE
#define STDEXT_ERROR_TITLE "(X)"
#endif

#ifndef STDEXT_SUCCESS_TITLE
#define STDEXT_SUCCESS_TITLE "(✓)"
#endif

#ifdef DEBUG
#define STDEXT_DEBUG_MODE true
#else
#define STDEXT_DEBUG_MODE false
#endif

namespace stdext {
    namespace logger {
        FILE* get_verbose_sink();

        void set_verbose_sink(FILE* sink);
        void add_sink(FILE* sink);
        void remove_sink(FILE* sink);
        void clear_sinks();
    }

    enum class confirm_default {
        yes,
        no
    };

    std::string log_body(const std::string& message, char end_char);
    std::string log_body(const std::string& title, const std::string& message, char end_char);

    void logv(const std::string& message);
    void logf(const std::string& message);
    void logf(ansi_color color, const std::string& message);

    template <typename... Args>
    void log(std::format_string<Args...> fmt, Args&&... args) {
        auto title = fg_color(ansi_color::bright_blue, STDEXT_LOG_TITLE);
        auto message = fg_color(ansi_color::bright_white, std::format(fmt, std::forward<Args>(args)...));

        logf(log_body(title, message, '\n'));
    }

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        auto body = log_body(STDEXT_INFO_TITLE, message, '\n');

        logf(ansi_color::bright_magenta, body);
    }

    template <typename... Args>
    void update(std::format_string<Args...> fmt, Args&&... args) {
        auto title = fg_color(ansi_color::bright_blue, STDEXT_UPDATE_TITLE);
        auto message = std::format(fmt, std::forward<Args>(args)...);

        logf(log_body(title, message, '\r'));
    }

    template <typename... Args>
    void verbose(std::format_string<Args...> fmt, Args&&... args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        auto body = log_body(STDEXT_VERBOSE_TITLE, message, '\n');

        logv(body);
    }

    template <typename... Args>
    inline void debug(std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (STDEXT_DEBUG_MODE) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            auto body = log_body(STDEXT_INFO_TITLE, message, '\n');

            logf(ansi_color::bright_magenta, body);
        }
    }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        auto body = log_body(STDEXT_WARN_TITLE, message, '\n');

        logf(ansi_color::bright_yellow, body);
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        auto body = log_body(STDEXT_ERROR_TITLE, message, '\n');

        logf(ansi_color::red, body);
    }

    template <typename... Args>
    void success(std::format_string<Args...> fmt, Args&&... args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        auto body = log_body(STDEXT_SUCCESS_TITLE, message, '\n');

        logf(ansi_color::bright_green, body);
    }

    template <typename... Args>
    std::string prompt(std::format_string<Args...> fmt, Args&&... args) {
        auto title = fg_color(ansi_color::bright_blue, STDEXT_PROMPT_TITLE);
        auto message = std::format(fmt, std::forward<Args>(args)...);
        auto question = fg_color(ansi_color::bright_white, message);

        return input(log_body(title, question, 0));
    }

    template <typename... Args>
    bool confirm(confirm_default default_value, std::format_string<Args...> fmt, Args&&... args) {
        auto fallback = default_value == confirm_default::yes;
        auto hint = fallback ? "(Y/n)" : "(y/N)";

        auto title = fg_color(ansi_color::bright_blue, STDEXT_CONFIRM_TITLE);
        auto message = std::format(fmt, std::forward<Args>(args)...);
        auto question = fg_color(ansi_color::bright_white, std::format("{} {} ", message, hint));

        while (true) {
            auto userInput = string::to_lower(input(log_body(title, question, 0)));

            if (userInput.empty()) {
                return fallback;
            }

            if (userInput == "y" || userInput == "yes") {
                return true;
            }

            if (userInput == "n" || userInput == "no") {
                return false;
            }

            warn("Invalid input. Please enter 'y' or 'n'.");
        }
    }

    template <typename... Args>
    bool confirm(std::format_string<Args...> fmt, Args&&... args) {
        return confirm(confirm_default::yes, fmt, std::forward<Args>(args)...);
    }
}
