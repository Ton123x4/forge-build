#pragma once

#include <vector>

#include "scanner.hpp"
#include "token.hpp"

namespace stdext {
    class lazy_parser {
    protected:
        scanner& m_scan;
        token m_current;
        token m_previous;

    public:
        lazy_parser(scanner& scan);

        virtual ~lazy_parser() = default;

        bool is_end() const;

        const token& peek() const;

        const token& advance();

        bool check(tok_kind kind) const;

        bool match(tok_kind kind);

        const token& consume(tok_kind kind, const std::string& message);
    };

    class eager_parser {
    protected:
        std::vector<token> m_tokens;
        size_t m_position;

    public:
        eager_parser(scanner& scanner);

        virtual ~eager_parser() = default;

        bool is_end() const;

        const token& peek() const;

        const token& peek(size_t offset) const;

        token& advance();

        token& previous();

        bool check(tok_kind kind) const;

        bool match(tok_kind kind);

        token& consume(tok_kind kind, const std::string& message);
    };
}
