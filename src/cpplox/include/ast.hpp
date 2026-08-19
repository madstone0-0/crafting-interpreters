#pragma once

#include <queue>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <variant>

#include "expr.hpp"
#include "stmt.hpp"
#include "token.hpp"
#include "utils.hpp"

namespace lox::ast {

    inline constexpr auto LiteralPrinter = utils::overload{
        [](const lexer::StringLiteral& e) { return e; },
        [](const lexer::NumberLiteral& e) { return std::to_string(e); },
        [](const lexer::BoolLiteral& e) {
            return std::string{e ? "true" : "false"};
        },
        [](const lexer::Callable& e) { return e.name; },
        [](std::monostate) { return std::string{"nil"}; }};

    namespace detail {

        template <class... Exprs>
        std::string parenthesize(const std::string& name,
                                 const Exprs&... exprs) {
            std::string out = "(" + name;
            ((out += " " + astPrinter(exprs)), ...);
            out += ")";
            return out;
        }

    }

    std::string astPrinter(const StmtPtr&, int depth = 0);

    std::string astPrinter(const ExprPtr&, int depth = 0);

    // std::string rpnAstPrinter(const ExprPtr&);
}
