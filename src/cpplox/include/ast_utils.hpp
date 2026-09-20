#pragma once

#include <queue>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "token.hpp"
#include "utils.hpp"

namespace lox::ast {

    inline constexpr auto LiteralPrinter = utils::overload{
        [](const lexer::StringLiteral& e) { return e; },
        [](const lexer::NumberLiteral& e) { return std::to_string(e); },
        [](const lexer::BoolLiteral& e) {
            return std::string{e ? "true" : "false"};
        },
        [](const lexer::CallableLiteral& e) { return e.name; },
        [](const lexer::ClassLiteral& e) { return e->name(); },
        [](const lexer::InstanceLiteral& e) { return e->name(); },
        [](std::monostate) { return std::string{"nil"}; }};

    std::string astPrinter(const lexer::Tokens&, int = 0);

    std::string astPrinter(const std::vector<StmtPtr>&, int = 0);

    std::string astPrinter(const std::vector<ExprPtr>&, int = 0);

    std::string astPrinter(const lexer::Tokens&, const std::vector<StmtPtr>&,
                           int = 0);

    std::string astPrinter(const StmtPtr&, int = 0);

    std::string astPrinter(const ExprPtr&, int = 0);

    namespace detail {

        template <class T>
        void append(std::string& out, const T& value, bool& first) {
            if (!first) {
                out += ' ';
            }

            out += astPrinter(value);
            first = false;
        }

        template <class... Things>
        std::string parenthesize(const std::string& name,
                                 const Things&... things) {
            constexpr size_t n = sizeof...(things);
            std::string out = "(" + name + (n > 0 ? " " : "");
            bool first = true;

            (
                [&] {
                    const auto value = astPrinter(things);

                    if (value.empty()) return;

                    if (!first) out += ' ';

                    out += value;
                    first = false;
                }(),
                ...);

            out += ')';
            return out;
        }

    }

    ast::Function& parseMethod(const StmtPtr&);

    ast::Variable& parseSuper(const ExprPtr&);

    // std::string rpnAstPrinter(const ExprPtr&);
}
