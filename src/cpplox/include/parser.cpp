#include "parser.hpp"

namespace lox::parser {

    ParseError Parser::error(const lexer::Token& tok, const std::string& msg) {
        lox::Lox::error(tok, msg);
        return ParseError{tok, msg.c_str()};
    }

    bool Parser::isAtEnd() { return peek().type == lexer::TokenType::EOF_; }

    lexer::Token Parser::peek() { return tokens.at(curr); }

    lexer::Token Parser::advance() {
        if (!isAtEnd()) curr++;
        return prev();
    }

    bool Parser::check(lexer::TokenType type) {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    lexer::Token Parser::prev() { return tokens.at(curr - 1); }

    void Parser::sync() {
        using enum lexer::TokenType;

        advance();

        // Discards all tokens until it finds a statement boundary, i.e. where
        // a new statement begins
        while (!isAtEnd()) {
            if (prev().type == SEMICOLON) return;

            switch (peek().type) {
                case CLASS:
                case FUN:
                case VAR:
                case FOR:
                case IF:
                case WHILE:
                case PRINT:
                case RETURN:
                    return;
            }

            advance();
        }
    }

    lexer::Token Parser::consume(lexer::TokenType type,
                                 const std::string& msg) {
        if (check(type)) return advance();

        throw error(peek(), msg);
    }

    ast::ExprPtr Parser::expression() { return equality(); }

    ast::ExprPtr Parser::equality() {
        using enum lexer::TokenType;
        return leftAss([this] { return comparison(); }, BANG_EQUAL,
                       EQUAL_EQUAL);
    }

    ast::ExprPtr Parser::comparison() {
        using enum lexer::TokenType;
        return leftAss([this] { return term(); }, GREATER, GREATER_EQUAL, LESS,
                       LESS_EQUAL);
    }

    ast::ExprPtr Parser::term() {
        using enum lexer::TokenType;
        return leftAss([this] { return factor(); }, PLUS, MINUS);
    }

    ast::ExprPtr Parser::factor() {
        using enum lexer::TokenType;
        return leftAss([this] { return unary(); }, STAR, SLASH);
    }

    ast::ExprPtr Parser::unary() {
        using enum lexer::TokenType;

        if (match(BANG, MINUS)) {
            auto op = prev();
            auto right = unary();
            return box(ast::Unary{
                .op = op,
                .right = std::move(right),
            });
        }

        return primary();
    }

    ast::ExprPtr Parser::primary() {
        using enum lexer::TokenType;

        if (match(FALSE)) return box(ast::Literal{false});
        if (match(TRUE)) return box(ast::Literal{true});
        if (match(NIL)) return box(ast::Literal{std::monostate{}});

        if (match(NUMBER, STRING)) return box(ast::Literal{prev().literal});

        if (match(LEFT_PAREN)) {
            auto expr = expression();
            consume(RIGHT_PAREN, "Expect ')' after expression.");
            return box(ast::Grouping{.expr = std::move(expr)});
        }

        throw error(peek(), "Expect expression");
    }
}
