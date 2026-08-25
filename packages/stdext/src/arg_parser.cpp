#include "stdext/arg_parser.hpp"

#include <format>

namespace stdext {
    arg_parser::arg_parser(int argc, char* argv[]) : counter(0) {
        for (int i = 1; i < argc; i++) {
            arguments.push_back(argv[i]);
        }
    }

    bool arg_parser::is_end() const {
        return counter >= arguments.size();
    }

    bool arg_parser::check(std::initializer_list<std::string> args) const {
        if (is_end()) {
            return false;
        }

        const auto& current = arguments.at(counter);

        for (const auto& arg : args) {
            if (current == arg) {
                return true;
            }
        }

        return false;
    }

    bool arg_parser::match(std::initializer_list<std::string> args) {
        if (!check(args)) {
            return false;
        }

        next();
        return true;
    }

    char* arg_parser::expect(const std::string& error_message) {
        if (is_end()) {
            throw missing_argument(error_message.data());
        }

        return arguments[counter++];
    }

    char* arg_parser::optional() {
        if (is_end()) {
            return nullptr;
        }

        return next();
    }

    char* arg_parser::next() {
        if (is_end()) {
            throw arg_parser_error(std::format("Expected an argument at position {} but reached end of input", counter));
        }

        return arguments[counter++];
    }

    char* arg_parser::current() const {
        if (counter == 0) {
            throw arg_parser_error("No argument has been consumed yet");
        }

        return arguments[counter - 1];
    }

    char* arg_parser::peek() const {
        if (is_end()) {
            arg_parser_error(std::format("Expected an argument at position {} but reached end of input", counter));
        }

        return arguments[counter];
    }

    void arg_parser::back() {
        if (counter == 0) {
            throw arg_parser_error("Cannot move back before first argument");
        }

        counter--;
    }
}
