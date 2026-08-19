#include "interpreter.hpp"

#include <algorithm>
#include <memory>
#include <variant>

#include "token.hpp"

namespace lox::interpreter {

    bool Interpreter::isTruthy(const lexer::Literal& l) {
        return std::visit(
            utils::overload{[](const lexer::StringLiteral&) { return true; },
                            [](const lexer::Identifer&) { return true; },
                            [](const lexer::NumberLiteral&) { return true; },
                            [](const lexer::BoolLiteral& b) { return b; },
                            [](std::monostate) { return false; }},
            l);
    }

    bool Interpreter::isEqual(const lexer::Literal& a,
                              const lexer::Literal& b) {
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

    lexer::Literal Interpreter::eval(const ast::Print& s) {
        auto val = visit(s.expr);
        std::println("{}", std::visit(ast::LiteralPrinter, val));
        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::Var& s) {
        lexer::Literal val{std::monostate{}};
        if (s.init) {
            val = visit(s.init);
        }

        env->define(s.name.lexeme, val);
        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::Block& s,
                                     const environment::EnvPtr& env) {
        auto prev = this->env;
        utils::ScopeGuard guard{[this, prev] { this->env = prev; }};

        this->env = env;
        for (const auto& stmt : s.stmts) {
            exec(stmt);
        }
        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::If& s) {
        if (isTruthy(visit(s.condition))) {
            exec(s.thenBr);
        } else if (s.elseBr) {
            exec(s.elseBr);
        }
        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::While& s) {
        while (isTruthy(visit(s.condition))) {
            exec(s.body);
        }
        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::For&) {
        return std::monostate{};
    }

    lexer::Literal Interpreter::visit(const ast::StmtPtr& stmt) {
        return std::visit(
            utils::overload{
                [this](const ast::Print& s) { return eval(s); },
                [this](const ast::Expression& s) { return visit(s.expr); },
                [this](const ast::If& s) { return eval(s); },
                [this](const ast::While& s) { return eval(s); },
                [this](const ast::For& s) { return eval(s); },
                [this](const ast::Var& s) { return eval(s); },
                [this](const ast::Block& s) {
                    return eval(
                        s, std::make_shared<environment::Environment>(env));
                },
            },
            *stmt);
    }

    lexer::Literal Interpreter::eval(const ast::Unary& e) {
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

    lexer::Literal Interpreter::eval(const ast::Binary& e) {
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
                if (std::holds_alternative<lexer::StringLiteral>(left)) {
                    auto rightStr = std::visit(lexer::LiteralVisitor, right);
                    return std::get<lexer::StringLiteral>(left) + rightStr;
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
            case SLASH: {
                if (!(std::holds_alternative<lexer::NumberLiteral>(left) &&
                      std::holds_alternative<lexer::NumberLiteral>(right))) {
                    checkArithOp();
                }
                auto leftNum = std::get<lexer::NumberLiteral>(left);
                auto rightNum = std::get<lexer::NumberLiteral>(right);
                if (rightNum == 0)
                    throw error::RuntimeError{e.op, "Division by zero"};
                return leftNum / rightNum;
            }

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

    lexer::Literal Interpreter::eval(const ast::Ternary& e) {
        using enum lexer::TokenType;

        if (e.op1.type != QUESTION_MARK && e.op2.type != COMMA)
            throw error::RuntimeError{e.op1, "Invalid ternary operation"};

        if (isTruthy(visit(e.left))) {
            return visit(e.mid);
        } else {
            return visit(e.right);
        }
    }

    lexer::Literal Interpreter::eval(const ast::Logical& e) {
        auto left = visit(e.left);

        if (e.op.type == lexer::TokenType::OR) {
            if (isTruthy(left)) return left;
        } else {
            if (!isTruthy(left)) return left;
        }

        return visit(e.right);
    }

    lexer::Literal Interpreter::visit(const ast::ExprPtr& expr) {
        return std::visit(
            utils::overload{
                [this](const ast::Assign& e) {
                    auto val = visit(e.value);
                    env->assign(e.name, val);
                    return val;
                },
                [this](const ast::Binary& e) { return eval(e); },
                [this](const ast::Unary& e) { return eval(e); },
                [this](const ast::Ternary& e) { return eval(e); },
                [this](const ast::Grouping& e) { return visit(e.expr); },
                [this](const ast::Variable& e) { return env->get(e.name); },
                [this](const ast::Logical& e) { return eval(e); },
                [](const ast::Literal& e) { return e.val; },
            },
            *expr);
    }

    void Interpreter::exec(const ast::StmtPtr& stmt) {
        if (!stmt) return;
        auto res = visit(stmt);

#ifdef DEBUG
        std::println("{}", ast::astPrinter(stmt));
#endif
        if (std::holds_alternative<ast::Expression>(*stmt)) {
            std::println("{}", std::visit(ast::LiteralPrinter, res));
        }
    }

    void Interpreter::interpret(const parser::Statements& stmts) {
        try {
            for (const auto& stmt : stmts) exec(stmt);
        } catch (const error::RuntimeError& e) {
            lox::Lox::runtimeError(e);
        }
    }

}
