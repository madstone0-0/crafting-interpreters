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

#include "literal.hpp"
#include "lox.hpp"
#include "token.hpp"
#include "utils.hpp"

namespace lox::lexer {

    class Lexer {
       private:
        static const std::unordered_map<std::string, TokenType> keywords;

        std::string source;
        Tokens tokens;

        std::size_t start{};
        std::size_t curr{};
        int line{1};

        bool isDigit(char c);

        bool isAlpha(char c);

        bool isAlphaNumeric(char c);

        bool isAtEnd();

        bool match(char c);

        char peek();

        char peekNext();

        char advance();

        void string();

        void number();

        void identifer();

        void scanToken();

        void addToken(TokenType);

        void addToken(TokenType, const Literal&);

       public:
        Lexer(std::string_view source)
        : source{source} {}

        Tokens scanTokens();
    };
};
