#include "resolver.hpp"

namespace lox::resolver {
    void Resolver::beginScope() { scopes.emplace_back(); }

    void Resolver::endScope() {
        for (const auto& [name, info] : scopes.back()) {
            if (!info.used)
                lox::Lox::warning(info.tok, "Local variable not used.");
        }
        scopes.pop_back();
    }

    void Resolver::declare(const lexer::Token& tok) {
        if (scopes.empty()) return;

        auto& scope = scopes.back();
        if (scope.contains(tok.lexeme))
            lox::Lox::error(
                tok, "Already a variable with this name in this scope.");

        scope.emplace(tok.lexeme, VarInfo{
                                      .tok = tok,
                                      .defined = false,
                                      .used = false,
                                      .idx = scope.size(),
                                  });
        // scope[tok.lexeme] = false;
    }

    void Resolver::define(const lexer::Token& tok) {
        if (scopes.empty()) return;
        scopes.back()[tok.lexeme].defined = true;
    }

    void Resolver::eval(const ast::Var& s) {
        declare(s.name);
        if (s.init) {
            resolve(s.init);
        }
        define(s.name);
    }

    void Resolver::eval(const ast::Block& s) {
        beginScope();
        resolve(s.stmts);
        endScope();
    }

    void Resolver::eval(const ast::Function& s) {
        declare(s.name);
        define(s.name);

        resolveFunction(s, FunctionType::FUNCTION);
    }

    void Resolver::eval(const ast::Class& s) {
        auto enclosingClass = currentClass;
        currentClass = ClassType::CLASS;

        declare(s.name);
        define(s.name);

        beginScope();
        scopes.back().emplace("this", VarInfo{
                                          .tok = lexer::Token{},
                                          .defined = true,
                                          .used = true,
                                      });
        for (const auto& meth : s.methods) {
            auto& f = ast::parseMethod(meth);

            auto decl = FunctionType::METHOD;
            if (f.name.lexeme == "init") decl = FunctionType::INIT;
            resolveFunction(f, decl);
        }
        for (const auto& get : s.getters) {
            auto& f = ast::parseMethod(get);

            resolveFunction(f, FunctionType::METHOD);
        }
        endScope();

        currentClass = enclosingClass;
    }

    void Resolver::eval(const ast::Return& s) {
        if (currentFunction == FunctionType::NONE)
            lox::Lox::error(s.keyword, "Can't return from top-level code.");

        if (s.val) {
            if (currentFunction == FunctionType::INIT)
                lox::Lox::error(s.keyword,
                                "Can't return a value from an initializer");
            resolve(s.val);
        }
    }

    void Resolver::resolve(const ast::StmtPtr& s) {
        if (!s) return;
        std::visit(utils::overload{
                       [this](const ast::Print& s) { resolve(s.expr); },
                       [this](const ast::Expression& s) { resolve(s.expr); },
                       [this](const ast::If& s) {
                           resolve(s.condition);
                           resolve(s.thenBr);
                           if (s.elseBr) resolve(s.elseBr);
                       },
                       [this](const ast::While& s) {
                           resolve(s.condition);
                           resolve(s.body);
                       },
                       [](const ast::For&) {
                           // resolve(s.init);
                           // resolve(s.condition);
                           // resolve(s.inc);
                       },
                       [](const ast::Break&) {},
                       [this](const ast::Var& s) { eval(s); },
                       [this](const ast::Block& s) { eval(s); },
                       [this](const ast::Function& s) { eval(s); },
                       [this](const ast::Class& s) { eval(s); },
                       [this](const ast::Return& s) { eval(s); },
                   },
                   *s);
    }

    void Resolver::eval(const ast::Call& e) {
        resolve(e.callee);

        for (const auto& arg : e.args) resolve(arg);
    }

    void Resolver::eval(const ast::Expr& self, const ast::Assign& e) {
        resolve(e.val);
        resolveLocal(self, e.name);
    }

    void Resolver::eval(const ast::Expr& self, const ast::Variable& e) {
        if (!scopes.empty()) {
            const auto& scope = scopes.back();
            auto itr = scope.find(e.name.lexeme);
            auto found = itr != scope.end();
            if (found && !itr->second.defined) {
                lox::Lox::error(
                    e.name,
                    "Can't read local variable in its own initializer");
            }
        }

        resolveLocal(self, e.name);
    }

    void Resolver::eval(const ast::Lambda& e) {
        auto enclosingFunction = currentFunction;
        currentFunction = FunctionType::FUNCTION;

        beginScope();
        for (const auto& param : e.params) {
            declare(param);
            define(param);
        }
        resolve(e.body);
        endScope();

        currentFunction = enclosingFunction;
    }

    void Resolver::eval(const ast::Expr& self, const ast::This& e) {
        if (currentClass == ClassType::NONE)
            lox::Lox::error(e.keyword, "Can't use 'this' outside of a class");

        resolveLocal(self, e.keyword);
    }

    void Resolver::resolveFunction(const ast::Function& f, FunctionType type) {
        auto enclosingFunction = currentFunction;
        currentFunction = type;

        beginScope();
        for (const auto& param : f.params) {
            declare(param);
            define(param);
        }
        resolve(f.body);
        endScope();
        currentFunction = enclosingFunction;
    }

    void Resolver::resolveLocal(const ast::Expr& e, const lexer::Token& tok) {
        for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
            if (auto& scope = scopes.at(i); scope.contains(tok.lexeme)) {
                auto& var = scope[tok.lexeme];
                interp->resolve(e, static_cast<int>(scopes.size()) - 1 - i,
                                var.idx);
                var.used = true;
                return;
            }
        }
    }

    void Resolver::resolve(const ast::ExprPtr& e) {
        if (!e) return;
        const auto& self = *e;
        std::visit(
            utils::overload{
                [this, &self](const ast::Assign& e) { eval(self, e); },
                [this](const ast::Binary& e) {
                    resolve(e.left);
                    resolve(e.right);
                },
                [this](const ast::Unary& e) { resolve(e.right); },
                [this](const ast::Ternary& e) {
                    resolve(e.left);
                    resolve(e.mid);
                    resolve(e.right);
                },
                [this](const ast::Grouping& e) { resolve(e.expr); },
                [this, &self](const ast::Variable& e) { eval(self, e); },
                [this](const ast::Logical& e) {
                    resolve(e.left);
                    resolve(e.right);
                },
                [this](const ast::Call& e) { eval(e); },
                [this](const ast::Get& e) { resolve(e.obj); },
                [this](const ast::Set& e) {
                    resolve(e.val);
                    resolve(e.obj);
                },
                [this, &self](const ast::This& e) { eval(self, e); },
                [this](const ast::Lambda& e) { eval(e); },
                [](const ast::Literal&) {},
            },
            *e);
    }

    void Resolver::resolve(const parser::Statements& stmts) {
        for (const auto& s : stmts) {
            if (!s) continue;
            resolve(s);
        }
    }
}
