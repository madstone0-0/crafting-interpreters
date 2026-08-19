#pragma once

#include <functional>
#include <optional>
#include <print>
#include <stdexcept>
#include <type_traits>
#include <variant>

#include "environment.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "lox.hpp"
#include "parser.hpp"
#include "stmt.hpp"
#include "token.hpp"

namespace lox::interpreter {

    class Interpreter {
       private:
        environment::EnvPtr env{std::make_shared<environment::Environment>()};

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

        lexer::Literal eval(const ast::Print&);
        lexer::Literal eval(const ast::Var&);
        lexer::Literal eval(const ast::Block&, const environment::EnvPtr&);
        lexer::Literal eval(const ast::If&);
        lexer::Literal eval(const ast::While&);

        lexer::Literal visit(const ast::StmtPtr&);

        lexer::Literal eval(const ast::Unary&);
        lexer::Literal eval(const ast::Binary&);
        lexer::Literal eval(const ast::Ternary&);
        lexer::Literal eval(const ast::Logical&);

        lexer::Literal visit(const ast::ExprPtr&);

        void exec(const ast::StmtPtr&);

       public:
        void interpret(const parser::Statements&);
    };

};
