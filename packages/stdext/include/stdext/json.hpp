#pragma once

#include "memory.hpp"
#include "hash_map.hpp"
#include "scanner.hpp"
#include "token.hpp"
#include <string>
#include <vector>

namespace stdext {
    enum struct json_type {
        null,
        boolean,
        number,
        string,
        array,
        object
    };

    struct json_node : public stdext::safe_object<json_node> {
        const json_type type;

        json_node(json_type type)
            : type(type) {}

        virtual ~json_node() = default;
    };

    struct json_boolean : public stdext::safe_object<json_boolean, json_node> {
        bool value;

        json_boolean(bool value)
            : safe_object(json_type::boolean), value(value) {}
    };

    struct json_number : public stdext::safe_object<json_number, json_node> {
        double value;

        json_number(double value)
            : safe_object(json_type::number), value(value) {}
    };

    struct json_string : public stdext::safe_object<json_string, json_node> {
        std::string value;

        json_string(const std::string& value)
            : safe_object(json_type::string), value(value) {}
    };

    struct json_array : public stdext::safe_object<json_array, json_node> {
        std::vector<json_node::safe_ptr> values;

        json_array()
            : safe_object(json_type::array) {}
    };

    struct json_object : public stdext::safe_object<json_object, json_node> {
        hash_map<std::string, json_node::safe_ptr> values;

        json_object()
            : safe_object(json_type::object) {}
    };

    struct json_scanner_error : public stdext::scanner_error {
        explicit json_scanner_error(const std::string& message)
            : stdext::scanner_error(message) {}

        explicit json_scanner_error(const tok_location& location, const std::string& message)
            : stdext::scanner_error(location, message) {}
    };

    struct json_syntax_error : public stdext::syntax_error {
        explicit json_syntax_error(const stdext::token& token, const std::string& message)
            : stdext::syntax_error(token, message) {}
    };

    namespace json {
        json_node::safe_ptr parse(const std::string& json_text);

        json_node::safe_ptr parse_file(const std::string& filepath);

        std::string escape_string(const std::string& string);

        std::string dump(const json_node::safe_ptr& tree, int indent = 0, int depth = 0);

        void dump_file(const json_node::safe_ptr& tree, const std::string& filepath);
    }
}
