#pragma once

#include <format>
#include <functional>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include "utils.hpp"

namespace lox::parser {
    using ast::box;
    using Statements = std::vector<ast::StmtPtr>;
    using Expressions = std::vector<ast::ExprPtr>;
    using Methods = std::vector<ast::StmtPtr>;

    class Parser {
       private:
        lexer::Tokens tokens;
        size_t curr{};
        size_t loopDepth{};

        error::ParseError error(const lexer::Token&, const std::string&);

        bool isAtEnd();

        lexer::Token peek();

        lexer::Token peekNext();

        lexer::Token advance();

        lexer::Token prev();

        bool check(lexer::TokenType type);

        bool checkNext(lexer::TokenType type);

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

        ast::StmtPtr classDecl();

        /**
         * @brief varDecl-> "var" IDENTIFIER ( "=" expression )? ";" ;
         */
        ast::StmtPtr varDecl();

        std::pair<lexer::Tokens, Statements> functionBody(
            std::string_view kind);

        /**
         * @brief function -> IDENTIFIER "(" parameters? ")" block ;
         */
        ast::StmtPtr function(std::string_view kind);

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
         * @brief breakStmt -> "break" ";" ;
         */
        ast::StmtPtr breakStmt();

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
         * @brief returnStmt -> "return" expression? ";" ;
         */
        ast::StmtPtr returnStmt();

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
         * @brief assignment -> ( call "." )? IDENTIFIER "=" assignment |
         * logic_or ;
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
         * @brief arguments -> expression ( "," expression )* ;
         */
        ast::ExprPtr arguments(ast::ExprPtr&&);

        /**
         * @brief call -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
         */
        ast::ExprPtr call();

        /**
         * @brief primary -> STRING | NUMBER | "true" | "false" | "nil" | "("
         * expression ")" | IDENTIFIER | lambda ;
         */
        ast::ExprPtr primary();

        /**
         * @brief lambda -> "fun" "(" parameters? ")" block;
         */
        ast::ExprPtr lambda();

       public:
        Parser(lexer::Tokens tok)
        : tokens{std::move(tok)} {}

        Statements parse();
    };

}
