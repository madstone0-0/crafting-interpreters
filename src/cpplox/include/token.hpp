#pragma once

#include <cctype>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "token.hpp"
#include "utils.hpp"

namespace lox::lexer {

    enum class TokenType : std::uint8_t {
        // Single-character tokens
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        COMMA,
        DOT,
        MINUS,
        PLUS,
        SEMICOLON,
        SLASH,
        STAR,
        QUESTION_MARK,
        COLON,

        // One or two character tokens
        BANG,
        BANG_EQUAL,
        EQUAL,
        EQUAL_EQUAL,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,

        // Literals
        IDENTIFER,
        STRING,
        NUMBER,

        // Keywords
        AND,
        CLASS,
        ELSE,
        FALSE,
        TRUE,
        IF,
        FUN,
        FOR,
        NIL,
        OR,
        PRINT,
        RETURN,
        SUPER,
        THIS,
        VAR,
        WHILE,
        BREAK,

        EOF_
    };

    struct Identifer {
        std::string name;
    };

    using StringLiteral = std::string;
    using NumberLiteral = double;
    using BoolLiteral = bool;

    using LiteralVariant = std::variant<std::monostate, StringLiteral,
                                        NumberLiteral, BoolLiteral>;
    using Literal = LiteralVariant;

    inline constexpr auto LiteralVisitor = utils::overload{
        [](const lexer::StringLiteral& e) { return e; },
        [](const lexer::NumberLiteral& e) { return std::to_string(e); },
        [](const lexer::BoolLiteral& e) {
            return std::string{e ? "true" : "false"};
        },
        [](std::monostate) { return std::string{""}; }};

    struct Token {
        const TokenType type{};
        const std::string lexeme;
        const Literal literal;
        const int line{};

        operator std::string() const {
            return std::format("{} {} {}", lox::utils::enumToString(type),
                               lexeme, std::visit(LiteralVisitor, literal));
        }
    };

    using Tokens = std::vector<Token>;
}
