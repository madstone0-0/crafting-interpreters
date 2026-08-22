#pragma once

#include <experimental/memory>
#include <vector>

#include "ast.hpp"
#include "interpreter.hpp"
#include "parser.hpp"

namespace lox::resolver {
    using InterpreterRef =
        std::experimental::observer_ptr<interpreter::Interpreter>;
    using Scope = std::unordered_map<std::string, bool>;
    using Scopes = std::vector<Scope>;

    enum class FunctionType : uint8_t { NONE, FUNCTION };

    class Resolver {
       private:
        InterpreterRef interp;
        Scopes scopes;
        FunctionType currentFunction = FunctionType::NONE;

        void beginScope();
        void endScope();

        void declare(const lexer::Token& tok);
        void define(const lexer::Token& tok);

        void eval(const ast::Var&);
        void eval(const ast::Block&);
        void eval(const ast::Function&);

        void resolve(const ast::StmtPtr&);

        void eval(const ast::Call&);
        void eval(const ast::Expr&, const ast::Assign&);
        void eval(const ast::Expr&, const ast::Variable&);
        void eval(const ast::Lambda&);

        void resolveFunction(const ast::Function&, FunctionType);
        void resolveLocal(const ast::Expr&, const lexer::Token&);
        void resolve(const ast::ExprPtr&);

       public:
        Resolver(InterpreterRef interp)
        : interp{interp} {}

        void resolve(const parser::Statements&);
    };

}
