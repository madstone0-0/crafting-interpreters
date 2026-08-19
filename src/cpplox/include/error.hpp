#pragma once

#include <stdexcept>

#include "literal.hpp"
#include "token.hpp"

namespace lox::error {

    struct ParseError : std::runtime_error {
        lexer::Token tok;
        ParseError(lexer::Token tok, const char* msg)
        : std::runtime_error(msg)
        , tok{std::move(tok)} {}
    };

    struct RuntimeError : std::runtime_error {
        const lexer::Token token;
        RuntimeError(lexer::Token tok, const char* msg)
        : runtime_error{msg}
        , token{std::move(tok)} {}
    };

    struct BreakError : std::exception {};

    struct Return : std::exception {
        const lexer::Literal val;

        Return(lexer::Literal val)
        : val{std::move(val)} {}
    };
}
