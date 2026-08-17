#pragma once

#include <functional>
#include <stdexcept>
#include <utility>

#include "ast.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "lexer.hpp"
#include "token.hpp"

namespace lox::parser {
    using ast::box;

    class Parser {
       private:
        lexer::Tokens tokens;
        size_t curr{};

        error::ParseError error(const lexer::Token&, const std::string&);

        bool isAtEnd();

        lexer::Token peek();

        lexer::Token advance();

        lexer::Token prev();

        bool check(lexer::TokenType type);

        void sync();

        lexer::Token consume(lexer::TokenType, const std::string&);

        template <class... Types>
        bool match(Types... types) {
            for (const auto& type : {types...}) {
                if (check(type)) {
                    advance();
                    return true;
                }
            }

            return false;
        }

        template <class... Types>
        ast::ExprPtr leftAss(const std::function<ast::ExprPtr()>& f,
                             Types... types) {
            auto expr = f();

            while (match(types...)) {
                auto op = prev();
                auto right = f();
                expr = box(ast::Binary{
                    .left = std::move(expr),
                    .op = op,
                    .right = std::move(right),
                });
            }

            return expr;
        }

        /**
         * @brief expression -> comma
         */
        ast::ExprPtr expression();

        /**
         * @brief comma -> "(" equality ( ","  equality )* ")" ;
         */
        ast::ExprPtr comma();

        ast::ExprPtr ternary();

        /**
         * @brief equality -> comparison ( ( "==" | "!=" ) comparison )* ;
         */
        ast::ExprPtr equality();

        /**
         * @brief comparison -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
         */
        ast::ExprPtr comparison();

        /**
         * @brief term -> factor ( ("+" | "-" ) factor )* ;
         */
        ast::ExprPtr term();

        /**
         * @brief factor -> unary ( ( "/" | "*" ) unary )* ;
         */
        ast::ExprPtr factor();

        /**
         * @brief unary -> ("!" | "-") unary | primary ;
         */
        ast::ExprPtr unary();

        /**
         * @brief primary -> STRING | NUMBER | "true" | "false" | "nil" | "("
         * expression ")" ;
         */
        ast::ExprPtr primary();

       public:
        Parser(lexer::Tokens tok)
        : tokens{std::move(tok)} {}

        ast::ExprPtr parse() {
            try {
                return expression();
            } catch (const error::ParseError& e) {
                return nullptr;
            }
        }
    };

}
