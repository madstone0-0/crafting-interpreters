#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <stdexcept>
#include <type_traits>
#include <variant>

#include "ast.hpp"
#include "ast_utils.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "literal.hpp"
#include "lox.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace lox::interpreter {
    using Locals = std::unordered_map<const ast::Expr*, int>;

    class Interpreter {
       private:
        environment::EnvPtr globals{
            std::make_shared<environment::Environment>()};
        Locals locals;
        environment::EnvPtr env{globals};
        parser::Statements stmts;

        bool isTruthy(const lexer::Literal&);

        bool isEqual(const lexer::Literal&, const lexer::Literal&);

        template <class... Operands>
        void checkNumberOperand(const lexer::Token& op,
                                const Operands&... operands) {
            bool all{true};
            for (const auto& operand : {operands...}) {
                if (!std::holds_alternative<lexer::NumberLiteral>(operand)) {
                    all = false;
                    break;
                }
            }
            if (!all) throw error::RuntimeError(op, "Operand must be number");
        }

        template <class T, class R = T>
        using BinaryFn = std::function<R(T a, T b)>;

        template <class T, class R>
        R binary(const lexer::Literal& a, const lexer::Literal& b,
                 const BinaryFn<T, R>& f,
                 const std::function<void()>& onFail) {
            if (!(std::holds_alternative<T>(a) &&
                  std::holds_alternative<T>(b))) {
                onFail();
            }
            return f(std::get<T>(a), std::get<T>(b));
        }

        lexer::Literal lookupVar(const lexer::Token& tok, const ast::Expr& e);

        lexer::Literal eval(const ast::Print&);
        lexer::Literal eval(const ast::Var&);
        lexer::Literal eval(const ast::Block&, const environment::EnvPtr&);
        lexer::Literal eval(const ast::If&);
        lexer::Literal eval(const ast::While&);
        lexer::Literal eval(const ast::Function&);

        lexer::Literal visit(const ast::StmtPtr&);

        lexer::Literal eval(const ast::Unary&);
        lexer::Literal eval(const ast::Binary&);
        lexer::Literal eval(const ast::Ternary&);
        lexer::Literal eval(const ast::Logical&);
        lexer::Literal eval(const ast::Call&);
        lexer::Literal eval(const ast::Lambda&);

        lexer::Literal visit(const ast::ExprPtr&);

        void exec(const ast::StmtPtr&, bool repl = false);

       public:
        Interpreter();

        void interpret(parser::Statements, bool repl = false);

        void resolve(const ast::Expr&, int);
    };

};
