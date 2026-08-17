#include "interpreter.hpp"

namespace lox::interpreter {

    bool Interpreter::isTruthy(const lexer::Literal& l) const {
        return std::visit(
            utils::overload{[](const lexer::StringLiteral&) { return true; },
                            [](const lexer::Identifer&) { return true; },
                            [](const lexer::NumberLiteral&) { return true; },
                            [](const lexer::BoolLiteral& b) { return b; },
                            [](std::monostate) { return false; }},
            l);
    }

    bool Interpreter::isEqual(const lexer::Literal& a,
                              const lexer::Literal& b) const {
        return std::visit(
            [](const auto& a, const auto& b) -> bool {
                using A = std::decay_t<decltype(a)>;
                using B = std::decay_t<decltype(b)>;
                if constexpr (std::is_same_v<A, B>) {
                    return a == b;
                } else {
                    return false;
                }
            },
            a, b);
    }

    lexer::Literal Interpreter::eval(const ast::Unary& e) const {
        using enum lexer::TokenType;
        auto right = visit(e.right);

        switch (e.op.type) {
            case BANG:
                return !isTruthy(right);
            case MINUS:
                checkNumberOperand(e.op, right);
                return -std::get<lexer::NumberLiteral>(right);
        }

        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::Binary& e) const {
        using enum lexer::TokenType;
        auto left = visit(e.left);
        auto right = visit(e.right);

        const auto checkArithOp = [this, &e, &left, &right] {
            checkNumberOperand(e.op, left, right);
        };

        switch (e.op.type) {
            case PLUS:
                if (std::holds_alternative<lexer::NumberLiteral>(left) &&
                    std::holds_alternative<lexer::NumberLiteral>(right)) {
                    return std::get<lexer::NumberLiteral>(left) +
                           std::get<lexer::NumberLiteral>(right);
                }
                if (std::holds_alternative<lexer::StringLiteral>(left) &&
                    std::holds_alternative<lexer::StringLiteral>(right)) {
                    return std::get<lexer::StringLiteral>(left) +
                           std::get<lexer::StringLiteral>(right);
                }
                throw error::RuntimeError{
                    e.op, "Operands must be two numbers or strings"};
            case MINUS:
                return binary<lexer::NumberLiteral>(
                    left, right,
                    BinaryFn<lexer::NumberLiteral>{
                        [](auto a, auto b) { return a - b; }},
                    checkArithOp);
            case STAR:
                return binary<lexer::NumberLiteral>(
                    left, right,
                    BinaryFn<lexer::NumberLiteral>{
                        [](auto a, auto b) { return a * b; }},
                    checkArithOp);
            case SLASH:
                return binary<lexer::NumberLiteral>(
                    left, right,
                    BinaryFn<lexer::NumberLiteral>{
                        [](auto a, auto b) { return a / b; }},
                    checkArithOp);

            case GREATER:
                return binary<lexer::NumberLiteral, lexer::BoolLiteral>(
                    left, right, [](auto a, auto b) { return a > b; },
                    checkArithOp);
            case GREATER_EQUAL:
                return binary<lexer::NumberLiteral, lexer::BoolLiteral>(
                    left, right, [](auto a, auto b) { return a >= b; },
                    checkArithOp);
            case LESS:
                return binary<lexer::NumberLiteral, lexer::BoolLiteral>(
                    left, right, [](auto a, auto b) { return a < b; },
                    checkArithOp);
            case LESS_EQUAL:
                return binary<lexer::NumberLiteral, lexer::BoolLiteral>(
                    left, right, [](auto a, auto b) { return a <= b; },
                    checkArithOp);
            case EQUAL_EQUAL:
                return isEqual(left, right);
            case BANG_EQUAL:
                return !isEqual(left, right);

            case COMMA:
                return right;
        }

        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::Ternary& e) const {
        using enum lexer::TokenType;
        auto left = visit(e.left);
        auto mid = visit(e.mid);
        auto right = visit(e.right);

        if (e.op1.type != QUESTION_MARK && e.op2.type != COMMA)
            throw std::runtime_error{"Invalid ternary operation"};

        if (isTruthy(left)) {
            return mid;
        } else {
            return right;
        }
    }

    lexer::Literal Interpreter::visit(const ast::ExprPtr& expr) const {
        return std::visit(
            utils::overload{
                [this](const ast::Binary& e) { return eval(e); },
                [this](const ast::Unary& e) { return eval(e); },
                [this](const ast::Ternary& e) { return eval(e); },
                [this](const ast::Grouping& e) { return visit(e.expr); },
                [](const ast::Literal& e) { return e.val; },
            },
            *expr);
    }

    void Interpreter::interpret(const ast::ExprPtr& expr) const {
        try {
            auto res = visit(expr);
            std::println("{}", std::visit(ast::LiteralPrinter, res));
        } catch (const error::RuntimeError& e) {
            lox::Lox::runtimeError(e);
        }
    }

}
