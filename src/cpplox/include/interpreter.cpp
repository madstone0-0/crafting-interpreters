#include "interpreter.hpp"

#include <format>
#include <memory>

#include "environment.hpp"
#include "literal.hpp"

namespace lox::interpreter {

    bool Interpreter::isTruthy(const lexer::Literal& l) {
        return std::visit(
            utils::overload{[](const lexer::StringLiteral&) { return true; },
                            [](const lexer::NumberLiteral&) { return true; },
                            [](const lexer::BoolLiteral& b) { return b; },
                            [](const lexer::CallableLiteral&) { return true; },
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
        try {
            while (isTruthy(visit(s.condition))) {
                exec(s.body);
            }
        } catch (const error::BreakError&) {
            // Stop looping
        }
        return std::monostate{};
    }

    lexer::Literal Interpreter::eval(const ast::Function& s) {
        auto f = lexer::CallableLiteral{
            .name = std::format("<fn {}>", s.name.lexeme),
            .arity_ = s.params.size(),
            .closure = std::make_shared<environment::Environment>(env),
            .f = [this, &s](const environment::EnvPtr& env,
                            const lexer::Literals& args) -> lexer::Literal {
                for (size_t i{}; i < s.params.size(); ++i) {
                    env->define(s.params.at(i).lexeme, args.at(i));
                }

                auto prevEnv = this->env;
                utils::ScopeGuard guard{
                    [this, prevEnv] { this->env = prevEnv; }};
                this->env = env;

                try {
                    for (const auto& block : s.body) {
                        exec(block);
                    }
                } catch (const error::Return& val) {
                    return val.val;
                }
                return std::monostate{};
            },
        };
        env->define(s.name.lexeme, f);

        return std::monostate{};
    }

    lexer::Literal Interpreter::visit(const ast::StmtPtr& stmt) {
        return std::visit(
            utils::overload{
                [this](const ast::Print& s) { return eval(s); },
                [this](const ast::Expression& s) { return visit(s.expr); },
                [this](const ast::If& s) { return eval(s); },
                [this](const ast::While& s) { return eval(s); },
                [](const ast::For&) {
                    return lexer::Literal{std::monostate{}};
                },
                [](const ast::Break&) {
                    throw error::BreakError{};
                    return lexer::Literal{std::monostate{}};
                },
                [this](const ast::Var& s) { return eval(s); },
                [this](const ast::Block& s) {
                    return eval(
                        s, std::make_shared<environment::Environment>(env));
                },
                [this](const ast::Function& s) { return eval(s); },
                [this](const ast::Return& s) {
                    lexer::Literal val{std::monostate{}};
                    if (s.val) val = visit(s.val);
                    throw error::Return{val};
                    return lexer::Literal{std::monostate{}};
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

    lexer::Literal Interpreter::eval(const ast::Call& e) {
        auto callee = visit(e.callee);

        lexer::Literals args;
        for (const auto& arg : e.args) {
            args.emplace_back(visit(arg));
        }

        if (!std::holds_alternative<lexer::CallableLiteral>(callee)) {
            throw error::RuntimeError{e.paren,
                                      "Can only call functions and classes"};
        }

        auto f = std::get<lexer::CallableLiteral>(callee);
        if (args.size() != f.arity()) {
            throw error::RuntimeError{
                e.paren, std::format("Expected {} arguments but got {}.",
                                     f.arity(), args.size())
                             .c_str()};
        }

        return f.call(std::make_shared<environment::Environment>(
                          f.closure ? f.closure : env),
                      args);
    }

    lexer::Literal Interpreter::visit(const ast::ExprPtr& expr) {
        return std::visit(
            utils::overload{
                [this](const ast::Assign& e) {
                    auto val = visit(e.val);
                    env->assign(e.name, val);
                    return val;
                },
                [this](const ast::Binary& e) { return eval(e); },
                [this](const ast::Unary& e) { return eval(e); },
                [this](const ast::Ternary& e) { return eval(e); },
                [this](const ast::Grouping& e) { return visit(e.expr); },
                [this](const ast::Variable& e) { return env->get(e.name); },
                [this](const ast::Logical& e) { return eval(e); },
                [this](const ast::Call& e) { return eval(e); },
                [](const ast::Literal& e) { return e.val; },
            },
            *expr);
    }

    void Interpreter::exec(const ast::StmtPtr& stmt, bool repl) {
        if (!stmt) return;

        auto res = visit(stmt);
        if (repl && std::holds_alternative<ast::Expression>(*stmt)) {
            std::println("{}", std::visit(ast::LiteralPrinter, res));
        }
    }

    Interpreter::Interpreter() {
        globals->define(
            "clock",
            lexer::CallableLiteral{
                .name = "<native fn>",
                .arity_ = 0,
                .f =
                    [](const environment::EnvPtr&, const lexer::Literals&) {
                        // Time in seconds since epoch
                        return lexer::Literal{
                            static_cast<lexer::NumberLiteral>(
                                std::chrono::duration_cast<
                                    std::chrono::milliseconds>(
                                    std::chrono::system_clock::now()
                                        .time_since_epoch())
                                    .count() /
                                1000.0)};
                    },
            });
        globals->define(
            "input",
            lexer::CallableLiteral{
                .name = "<native fn>",
                .arity_ = 0,
                .f =
                    [](const environment::EnvPtr&, const lexer::Literals&) {
                        std::string input;
                        std::getline(std::cin, input);
                        return lexer::Literal{lexer::StringLiteral{input}};
                    },
            });
        globals->define(
            "number",
            lexer::CallableLiteral{
                .name = "<native fn>",
                .arity_ = 1,
                .f = [](const environment::EnvPtr&,
                        const lexer::Literals& args) -> lexer::Literal {
                    auto arg = args.at(0);
                    return std::visit(
                        utils::overload{
                            [](const lexer::NumberLiteral& n) {
                                return lexer::Literal{n};
                            },
                            [](const lexer::StringLiteral& s) {
                                try {
                                    return lexer::Literal{std::stod(s)};
                                } catch (const std::invalid_argument&) {
                                    throw error::RuntimeError{
                                        {}, "Invalid number format"};
                                }
                            },
                            [](const lexer::BoolLiteral& b) {
                                return lexer::Literal{b ? 1.0 : 0.0};
                            },
                            [](const auto&) {
                                throw error::RuntimeError{
                                    {}, "Cannot convert to number"};
                                return lexer::Literal{std::monostate{}};
                            },
                        },
                        arg);
                },
            });
        globals->define(
            "string",
            lexer::CallableLiteral{
                .name = "<native fn>",
                .arity_ = 1,
                .f = [](const environment::EnvPtr&,
                        const lexer::Literals& args) -> lexer::Literal {
                    auto arg = args.at(0);
                    return std::visit(ast::LiteralPrinter, arg);
                },
            });
        globals->define(
            "bool",
            lexer::CallableLiteral{
                .name = "<native fn>",
                .arity_ = 1,
                .f = [this](const environment::EnvPtr&,
                            const lexer::Literals& args) -> lexer::Literal {
                    return isTruthy(args.at(0));
                },
            });
    }

    void Interpreter::interpret(parser::Statements sts, bool repl) {
        auto start = stmts.size();
        stmts.reserve(stmts.size() + sts.size());
        for (auto& stmt : sts) stmts.emplace_back(std::move(stmt));

        try {
            for (auto i{start}; i < stmts.size(); ++i) {
#ifdef DEBUG
                if (stmts[i]) std::println("{}", ast::astPrinter(stmts[i]));
#endif
                exec(stmts[i], repl);
            }
        } catch (const error::RuntimeError& e) {
            lox::Lox::runtimeError(e);
        }
    }

}
