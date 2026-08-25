#include "stdext/json.hpp"
#include "stdext/filesystem.hpp"
#include "stdext/scanner.hpp"
#include "stdext/parser.hpp"
#include "stdext/string.hpp"
#include "stdext/token.hpp"
#include <format>

namespace stdext::json {
    json_node::safe_ptr parse_node(stdext::lazy_parser& parser);

    json_node::safe_ptr parse_identifier(stdext::token& token) {
        if (token.value == "null") {
            return json_node::make(json_type::null);
        }

        if (token.value == "false") {
            return json_boolean::make(false);
        }

        if (token.value == "true") {
            return json_boolean::make(true);
        }

        throw json_syntax_error(token, "expected 'null', 'true' or 'false'");
    }

    json_node::safe_ptr parse_number(stdext::token& token) {
        return json_number::make(std::stod(token.value));
    }

    json_node::safe_ptr parse_string(stdext::token& token) {
        return json_string::make(token.value);
    }

    json_node::safe_ptr parse_array(stdext::lazy_parser& parser) {
        auto array_node = json_array::make();
        bool expects_value = false;

        while (!parser.is_end()) {
            auto token = parser.peek();

            switch (token.kind) {
                case stdext::tok_kind::close_bracket: {
                    parser.advance();

                    if (expects_value) {
                        throw json_syntax_error(token, "unexpected trailing comma before ']'");
                    }

                    return array_node;
                }

                case stdext::tok_kind::comma: {
                    parser.advance();

                    if (array_node->values.empty() || expects_value) {
                        throw json_syntax_error(token, "unexpected ',' in array");
                    }

                    expects_value = true;

                    break;
                }

                default: {
                    if (array_node->values.empty() || expects_value) {
                        array_node->values.push_back(parse_node(parser));

                        expects_value = false;

                        break;
                    }

                    throw json_syntax_error(token, "expected ',' or ']' after array element");
                }
            }
        }

        throw json_syntax_error(parser.peek(), "unterminated array, expected ']'");
    }

    json_node::safe_ptr parse_object(stdext::lazy_parser& parser) {
        auto object_node = json_object::make();
        bool expects_entry = false;

        while (!parser.is_end()) {
            auto token = parser.peek();

            switch (token.kind) {
                case stdext::tok_kind::close_brace: {
                    parser.advance();

                    if (expects_entry) {
                        throw json_syntax_error(token, "unexpected trailing comma before '}'");
                    }

                    return std::move(object_node);
                }

                case stdext::tok_kind::comma: {
                    parser.advance();

                    if (object_node->values.empty() || expects_entry) {
                        throw json_syntax_error(token, "unexpected ',' in object");
                    }

                    expects_entry = true;

                    break;
                }

                case stdext::tok_kind::string: {
                    if (!object_node->values.empty() && !expects_entry) {
                        throw json_syntax_error(token, "expected ',' or '}' after object entry");
                    }

                    parser.advance();

                    auto key = token.value;
                    auto colon = parser.advance();

                    if (colon.kind != stdext::tok_kind::colon) {
                        throw json_syntax_error(colon, "expected ':' after object key");
                    }

                    object_node->values.insert(key, std::move(parse_node(parser)));
                    expects_entry = false;

                    break;
                }

                default: {
                    throw json_syntax_error(token, "expected string key in object");
                }
            }
        }

        throw json_syntax_error(parser.peek(), "unterminated object, expected '}'");
    }

    json_node::safe_ptr parse_node(stdext::lazy_parser& parser) {
        auto token = parser.advance();

        switch (token.kind) {
            case stdext::tok_kind::identifier: {
                return parse_identifier(token);
            }

            case stdext::tok_kind::integer:
            case stdext::tok_kind::floating: {
                return parse_number(token);
            }

            case stdext::tok_kind::string: {
                return parse_string(token);
            }

            case stdext::tok_kind::open_bracket: {
                return parse_array(parser);
            }

            case stdext::tok_kind::open_brace: {
                return parse_object(parser);
            }

            default:
                throw json_syntax_error(token, "unexpected token, expected a JSON value");
        }
    }

    [[noreturn]] void raise_error(const stdext::scanner_error& error) {
        if (error.location) {
            throw json_scanner_error(error.location.value(), error.what());
        }

        throw json_scanner_error(error.what());
    }

    json_node::safe_ptr parse(const std::string& json_text) {
        try {
            auto scanner = stdext::scanner("root", json_text);
            auto parser = stdext::lazy_parser(scanner);

            return parse_node(parser);
        }
        catch (const stdext::scanner_error& error) {
            raise_error(error);
        }
    }

    json_node::safe_ptr parse_file(const std::string& filepath) {
        try {
            auto scanner = stdext::scanner(filepath);
            auto parser = stdext::lazy_parser(scanner);

            return parse_node(parser);
        }
        catch (const stdext::scanner_error& error) {
            raise_error(error);
        }
    }

    std::string escape_string(const std::string& string) {
        auto result = std::string();

        result.reserve(string.size());

        for (auto c : string) {
            switch (c) {
                case '"': {
                    result += "\\\"";
                    break;
                }

                case '\\': {
                    result += "\\\\";
                    break;
                }

                case '\b': {
                    result += "\\b";
                    break;
                }

                case '\f': {
                    result += "\\f";
                    break;
                }

                case '\n': {
                    result += "\\n";
                    break;
                }

                case '\r': {
                    result += "\\r";
                    break;
                }

                case '\t': {
                    result += "\\t";
                    break;
                }

                default: {
                    auto code = static_cast<unsigned char>(c);

                    if (code < 0x20) {
                        result += std::format("\\u{:04x}", code);
                    }
                    else {
                        result += c;
                    }

                    break;
                }
            }
        }

        return result;
    }

    std::string dump(const json_node::safe_ptr& tree, int indent, int depth) {
        if (!tree) {
            return "null";
        }

        auto pad_here = stdext::string::repeat(" ", indent > 0 ? (indent * depth) : 0);
        auto pad_inner = stdext::string::repeat(" ", indent > 0 ? (indent * (depth + 1)) : 0);
        auto colon_space = indent > 0 ? " " : "";
        auto newline = indent > 0 ? "\n" : "";

        switch (tree->type) {
            case json_type::null: {
                return "null";
            }

            case json_type::boolean: {
                auto* node = json_boolean::from(tree.get());

                return node->value ? "true" : "false";
            }

            case json_type::number: {
                auto* node = json_number::from(tree.get());

                return std::to_string(node->value);
            }

            case json_type::string: {
                auto* node = json_string::from(tree.get());

                return std::format("\"{}\"", escape_string(node->value));
            }

            case json_type::array: {
                auto* node = json_array::from(tree.get());

                if (node->values.empty()) {
                    return "[]";
                }

                auto result = std::string("[");
                bool first = true;

                result += newline;

                for (auto& item : node->values) {
                    if (!first) {
                        result += ",";
                        result += newline;
                    }

                    result += pad_inner;
                    result += dump(item, indent, depth + 1);

                    first = false;
                }

                result += newline;
                result += pad_here;
                result += "]";

                return result;
            }

            case json_type::object: {
                auto* node = json_object::from(tree.get());

                if (node->values.empty()) {
                    return "{}";
                }

                auto result = std::string("{");
                bool first = true;

                result += newline;

                for (const auto& entry : node->values) {
                    if (!first) {
                        result += ",";
                        result += newline;
                    }

                    result += pad_inner;
                    result += std::format("\"{}\":{}{}", escape_string(entry.first), colon_space, dump(entry.second, indent, depth + 1));

                    first = false;
                }

                result += newline;
                result += pad_here;
                result += "}";

                return result;
            }
        }

        return "null";
    }

    void dump_file(const json_node::safe_ptr& tree, const std::string& filepath) {
        stdext::fs::write_file(filepath, dump(tree));
    }

}
