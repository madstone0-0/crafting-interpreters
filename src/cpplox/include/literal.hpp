#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "environment_fwd.hpp"
#include "utils.hpp"
namespace lox::lexer {

    struct Callable;
    using StringLiteral = std::string;
    using NumberLiteral = double;
    using BoolLiteral = bool;
    using CallableLiteral = Callable;

    using LiteralVariant =
        std::variant<std::monostate, StringLiteral, NumberLiteral, BoolLiteral,
                     CallableLiteral>;
    using Literal = LiteralVariant;

    using Literals = std::vector<Literal>;

    using CallableFunc = std::function<lexer::Literal(
        const environment::EnvPtr&, const Literals&)>;

    struct Callable {
        std::string name;
        size_t arity_{};
        environment::EnvPtr closure;
        CallableFunc f;

        bool operator==(const Callable&) const { return false; }

        lexer::Literal call(const environment::EnvPtr&, const Literals&) const;

        auto arity() const { return arity_; }
    };

    inline constexpr auto LiteralVisitor = utils::overload{
        [](const StringLiteral& e) { return e; },
        [](const NumberLiteral& e) { return std::to_string(e); },
        [](const BoolLiteral& e) { return std::string{e ? "true" : "false"}; },
        [](const Callable& e) { return e.name; },
        [](std::monostate) { return std::string{""}; }};
}
