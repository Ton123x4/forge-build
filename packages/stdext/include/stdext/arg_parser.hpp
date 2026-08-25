#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <initializer_list>

namespace stdext {
    struct arg_parser_error : public std::runtime_error {
        explicit arg_parser_error(const std::string& message) :
            std::runtime_error(message) {}
    };

    struct missing_argument : std::runtime_error {
        explicit missing_argument(const std::string& argument) :
            std::runtime_error(argument) {}
    };

    class arg_parser {
    private:
        std::vector<char*> arguments;
        std::size_t counter;

    public:
        arg_parser(int argc, char* argv[]);

        bool is_end() const;
        bool check(std::initializer_list<std::string> args) const;
        bool match(std::initializer_list<std::string> args);

        char* expect(const std::string& error_message);
        char* current() const;
        char* peek() const;
        char* optional();
        char* next();

        void back();
    };
}
