#pragma once

#include <experimental/memory>
#include <vector>

#include "ast.hpp"
#include "interpreter.hpp"
#include "parser.hpp"

namespace lox::resolver {
    struct VarInfo {
        lexer::Token tok;
        bool defined{};
        bool used{};
        size_t idx{};
    };

    using InterpreterRef =
        std::experimental::observer_ptr<interpreter::Interpreter>;
    using Scope = std::unordered_map<std::string, VarInfo>;
    using Scopes = std::vector<Scope>;

    enum class FunctionType : uint8_t { NONE, FUNCTION, INIT, METHOD };

    enum class ClassType : uint8_t { NONE, CLASS, SUBCLASS };

    class Resolver {
       private:
        InterpreterRef interp;
        Scopes scopes;
        FunctionType currentFunction = FunctionType::NONE;
        ClassType currentClass = ClassType::NONE;

        void beginScope();
        void endScope();

        void declare(const lexer::Token& tok);
        void define(const lexer::Token& tok);

        void eval(const ast::Var&);
        void eval(const ast::Block&);
        void eval(const ast::Function&);
        void eval(const ast::Class&);
        void eval(const ast::Return&);

        void resolve(const ast::StmtPtr&);

        void eval(const ast::Call&);
        void eval(const ast::Expr&, const ast::Assign&);
        void eval(const ast::Expr&, const ast::Variable&);
        void eval(const ast::Lambda&);
        void eval(const ast::Expr&, const ast::This&);

        void resolveFunction(const ast::Function&, FunctionType);
        void resolveLocal(const ast::Expr&, const lexer::Token&);
        void resolve(const ast::ExprPtr&);

       public:
        Resolver(InterpreterRef interp)
        : interp{interp} {}

        void resolve(const parser::Statements&);
    };

}
