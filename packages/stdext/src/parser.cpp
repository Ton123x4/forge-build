#include "stdext/parser.hpp"

namespace stdext {
    // ---------------- lazy_parser ----------------

    lazy_parser::lazy_parser(scanner& scan) : m_scan(scan) {
        if (!is_end()) {
            advance();
        }
    }

    bool lazy_parser::is_end() const {
        return m_scan.is_eos();
    }

    const token& lazy_parser::peek() const {
        return m_current;
    }

    const token& lazy_parser::advance() {
        m_previous = m_current;
        m_current = m_scan.next_token();

        return m_previous;
    }

    bool lazy_parser::check(tok_kind kind) const {
        if (is_end()) {
            return false;
        }

        return m_current.kind == kind;
    }

    bool lazy_parser::match(tok_kind kind) {
        if (!check(kind)) {
            return false;
        }

        advance();
        return true;
    }

    const token& lazy_parser::consume(tok_kind kind, const std::string& message) {
        if (!match(kind)) {
            throw syntax_error(m_current, message);
        }

        return m_previous;
    }

    // ---------------- eager_parser ----------------

    eager_parser::eager_parser(scanner& scanner) : m_position(0) {
        while (!scanner.is_eos()) {
            m_tokens.push_back(scanner.next_token());
        }
    }

    bool eager_parser::is_end() const {
        return m_position >= m_tokens.size();
    }

    const token& eager_parser::peek() const {
        return m_tokens.at(m_position);
    }

    const token& eager_parser::peek(size_t offset) const {
        return m_tokens.at(m_position + offset);
    }

    token& eager_parser::advance() {
        return m_tokens[m_position++];
    }

    token& eager_parser::previous() {
        return m_tokens[m_position - 1];
    }

    bool eager_parser::check(tok_kind kind) const {
        if (is_end()) {
            return false;
        }

        return m_tokens[m_position].kind == kind;
    }

    bool eager_parser::match(tok_kind kind) {
        if (!check(kind)) {
            return false;
        }

        advance();
        return true;
    }

    token& eager_parser::consume(tok_kind kind, const std::string& message) {
        if (!match(kind)) {
            throw syntax_error(m_tokens[m_position], message);
        }

        return previous();
    }
}
