#pragma once

#include <format>
#include <variant>
#include <vector>

#include "utils.hpp"

namespace lox::value {
    using NumberLiteral = double;
    using Value = std::variant<NumberLiteral>;

    using Values = std::vector<Value>;

    inline constexpr auto ValueVisitor = utils::overload{
        [](const NumberLiteral& v) { return std::format("{:g}", v); },
    };

};

template <>
struct std::formatter<lox::value::Value> : std::formatter<std::string> {
    auto format(const lox::value::Value& val, std::format_context& ctx) const {
        return std::formatter<std::string>::format(
            std::visit(lox::value::ValueVisitor, val), ctx);
    }
};
