#pragma once

#include <stdexcept>

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
}
