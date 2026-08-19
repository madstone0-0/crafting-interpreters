#pragma once

#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "lexer.hpp"
#include "stmt.hpp"
#include "token.hpp"

namespace lox::parser {
    using ast::box;
    using Statements = std::vector<ast::StmtPtr>;

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
         * @brief declaration -> varDecl | statement ;
         */
        ast::StmtPtr declaration();

        /**
         * @brief varDecl-> "var" IDENTIFIER ( "=" expression )? ";" ;
         */
        ast::StmtPtr varDecl();

        /**
         * @brief statement -> exprStmt | printStmt;
         */
        ast::StmtPtr statement();

        /**
         * @brief exprStmt -> expression ";" ;
         */
        ast::StmtPtr exprStmt();

        /**
         * @brief printStmt -> "print" expression ";" ;
         */
        ast::StmtPtr printStmt();

        /**
         * @brief ifStmt -> "if" "(" expression ")" statement ( "else"
         * statement )? ;
         */
        ast::StmtPtr ifStmt();

        /**
         * @brief whileStmt -> "while" "(" expression ")" statement ;
         */
        ast::StmtPtr whileStmt();

        /**
         * @brief forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression?
         * ";" expression? ")" statement ;
         */
        ast::StmtPtr forStmt();

        /**
         * @brief block -> "{" declaration* "}" ;
         */
        Statements block();

        /**
         * @brief expression -> comma
         */
        ast::ExprPtr expression();

        /**
         * @brief comma -> assignment ( ","  assignment )*  ;
         */
        ast::ExprPtr comma();

        /**
         * @brief  assignment -> IDENTIFIER "=" assignment | ternary ;
         */
        ast::ExprPtr assignment();

        /**
         * @brief logic_or -> logic_and ( "or" logic_and )* ;
         */
        ast::ExprPtr logicOr();

        /**
         * @brief logic_and -> ternary ( "and" ternary)* ;
         */
        ast::ExprPtr logicAnd();

        /**
         * @brief ternary -> equality ( "?" expression ":" ternary )? ;
         */
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

        Statements parse();
    };

}
