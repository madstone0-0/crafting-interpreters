#pragma once

#include <functional>
#include <optional>
#include <print>
#include <stdexcept>
#include <type_traits>
#include <variant>

#include "expr.hpp"
#include "lox.hpp"
#include "token.hpp"
#include "error.hpp"

namespace lox::interpreter {

    class Interpreter {
       private:
        bool isTruthy(const lexer::Literal& l) const;

        bool isEqual(const lexer::Literal& a, const lexer::Literal& b) const;

        template <class... Operands>
        void checkNumberOperand(const lexer::Token& op,
                                const Operands&... operands) const {
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
                 const std::function<void()>& onFail) const {
            if (!(std::holds_alternative<T>(a) &&
                  std::holds_alternative<T>(b))) {
                onFail();
            }
            return f(std::get<T>(a), std::get<T>(b));
        }

        lexer::Literal eval(const ast::Unary& e) const;
        lexer::Literal eval(const ast::Binary& e) const;
        lexer::Literal eval(const ast::Ternary& e) const;

        lexer::Literal visit(const ast::ExprPtr& expr) const;

       public:
        void interpret(const ast::ExprPtr& expr) const;
    };

};
