#include "parser.hpp"

#include <variant>

#include "expr.hpp"
#include "stmt.hpp"
#include "token.hpp"

namespace lox::parser {

    error::ParseError Parser::error(const lexer::Token& tok,
                                    const std::string& msg) {
        lox::Lox::error(tok, msg);
        return error::ParseError{tok, msg.c_str()};
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

    ast::StmtPtr Parser::declaration() {
        using enum lexer::TokenType;

        try {
            if (match(VAR)) return varDecl();

            return statement();
        } catch (const error::ParseError& e) {
            sync();
            return nullptr;
        }
    }

    ast::StmtPtr Parser::varDecl() {
        using enum lexer::TokenType;

        auto name = consume(IDENTIFER, "Expect variable name.");

        ast::ExprPtr init{};
        if (match(EQUAL)) init = expression();

        consume(SEMICOLON, "Expect ';' after variable declaration.");

        return box(ast::Var{
            .name = name,
            .init = std::move(init),
        });
    }

    ast::StmtPtr Parser::statement() {
        using enum lexer::TokenType;
        if (match(IF)) return ifStmt();
        if (match(WHILE)) return whileStmt();
        if (match(FOR)) return forStmt();
        if (match(PRINT)) return printStmt();
        if (match(LEFT_BRACE)) return box(ast::Block{block()});
        return exprStmt();
    }

    ast::StmtPtr Parser::exprStmt() {
        using enum lexer::TokenType;

        auto expr = expression();
        consume(SEMICOLON, "Expect ';' after value");
        return box(ast::Expression{std::move(expr)});
    }

    ast::StmtPtr Parser::printStmt() {
        using enum lexer::TokenType;

        auto val = expression();
        consume(SEMICOLON, "Expect ';' after value");
        return box(ast::Print{std::move(val)});
    }

    ast::StmtPtr Parser::ifStmt() {
        using enum lexer::TokenType;
        consume(LEFT_PAREN, "Expect '(' after 'if'.");
        auto cond = expression();
        consume(RIGHT_PAREN, "Expect ')' after if condition.");

        auto thenBr = statement();
        ast::StmtPtr elseBr{nullptr};
        if (match(ELSE)) {
            elseBr = statement();
        }

        return box(ast::If{
            .condition = std::move(cond),
            .thenBr = std::move(thenBr),
            .elseBr = std::move(elseBr),
        });
    }

    ast::StmtPtr Parser::whileStmt() {
        using enum lexer::TokenType;
        consume(LEFT_PAREN, "Expect '(' after 'while'.");
        auto cond = expression();
        consume(RIGHT_PAREN, "Expect ')' after while loop.");

        auto body = statement();

        return box(ast::While{
            .condition = std::move(cond),
            .body = std::move(body),
        });
    }

    ast::StmtPtr Parser::forStmt() {
        using enum lexer::TokenType;
        consume(LEFT_PAREN, "Expect '(' after 'for'.");

        ast::StmtPtr init{};
        if (match(SEMICOLON))
            init = nullptr;
        else if (match(VAR))
            init = varDecl();
        else
            init = exprStmt();

        ast::ExprPtr cond{};
        if (!check(SEMICOLON)) cond = expression();

        consume(SEMICOLON, "Expect ';' after loop condition.");

        ast::ExprPtr inc{};
        if (!check(RIGHT_PAREN)) inc = expression();
        consume(RIGHT_PAREN, "Expect ')' after for clauses.");

        // Desugaring basically converts the for loop into it's equivalent
        // while loop, i.e.:
        // {
        //  INIT?
        //  while (COND || true) {
        //      BODY
        //      INC?
        //  }
        // }
        //
        auto body = statement();
        if (inc) {
            Statements s;
            s.emplace_back(std::move(body));
            s.emplace_back(box(ast::Expression{std::move(inc)}));

            body = box(ast::Block{.stmts = std::move(s)});
        }

        if (!cond) cond = box(ast::Literal{true});
        body = box(
            ast::While{.condition = std::move(cond), .body = std::move(body)});

        if (init) {
            Statements s;
            s.emplace_back(std::move(init));
            s.emplace_back(std::move(body));

            body = box(ast::Block{.stmts = std::move(s)});
        }

        return body;
    }

    Statements Parser::block() {
        using enum lexer::TokenType;
        Statements stmts;

        while (!check(RIGHT_BRACE) && !isAtEnd())
            stmts.emplace_back(declaration());

        consume(RIGHT_BRACE, "Expected '}' after block");
        return stmts;
    }

    ast::ExprPtr Parser::expression() { return assignment(); }

    ast::ExprPtr Parser::comma() {
        using enum lexer::TokenType;
        auto expr = ternary();

        while (match(COMMA)) {
            auto op = prev();
            auto right = ternary();
            expr = box(ast::Binary{
                .left = std::move(expr),
                .op = op,
                .right = std::move(right),
            });
        }

        return expr;
    }

    ast::ExprPtr Parser::assignment() {
        using enum lexer::TokenType;
        auto expr = logicOr();

        if (match(EQUAL)) {
            auto equals = prev();
            auto val = assignment();

            if (auto* variable = std::get_if<ast::Variable>(&(*expr))) {
                auto name = variable->name;
                return box(ast::Assign{
                    .name = name,
                    .value = std::move(val),
                });
            }

            error(equals, "Invalid assignment target.");
        }

        return expr;
    }

    ast::ExprPtr Parser::logicOr() {
        using enum lexer::TokenType;
        auto expr = logicAnd();

        while (match(OR)) {
            auto op = prev();
            auto right = logicAnd();
            expr = box(ast::Logical{
                .left = std::move(expr),
                .op = op,
                .right = std::move(right),
            });
        }

        return expr;
    }

    ast::ExprPtr Parser::logicAnd() {
        using enum lexer::TokenType;
        auto expr = ternary();

        while (match(AND)) {
            auto op = prev();
            auto right = ternary();
            expr = box(ast::Logical{
                .left = std::move(expr),
                .op = op,
                .right = std::move(right),
            });
        }

        return expr;
    }

    ast::ExprPtr Parser::ternary() {
        using enum lexer::TokenType;
        auto expr = equality();

        if (match(QUESTION_MARK)) {
            auto op1 = prev();
            auto mid = expression();
            auto op2 = consume(COLON, "Expect ':' after expression.");
            auto right = ternary();
            expr = box(ast::Ternary{
                .left = std::move(expr),
                .op1 = op1,
                .mid = std::move(mid),
                .op2 = op2,
                .right = std::move(right),
            });
        }

        return expr;
    }

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

        if (match(IDENTIFER)) return box(ast::Variable{prev()});

        if (match(LEFT_PAREN)) {
            auto expr = expression();
            consume(RIGHT_PAREN, "Expect ')' after expression.");
            return box(ast::Grouping{.expr = std::move(expr)});
        }

        throw error(peek(), "Expect expression");
    }

    Statements Parser::parse() {
        Statements stmts;

        while (!isAtEnd()) stmts.emplace_back(declaration());

        return stmts;
    }
}
