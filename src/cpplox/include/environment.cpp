#include "environment.hpp"

namespace lox::environment {

    void Environment::define(const std::string& name,
                             const lexer::Literal& val) {
        values.insert({name, val});
    }

    lexer::Literal Environment::get(const lexer::Token& name) {
        if (values.contains(name.lexeme)) return values.at(name.lexeme);

        if (enclosing) return enclosing->get(name);

        throw error::RuntimeError{
            name,
            std::format("Undefined variable '{}'.", name.lexeme).c_str()};
    }

    void Environment::assign(const lexer::Token& name,
                             const lexer::Literal& val) {
        if (values.contains(name.lexeme)) {
            values.at(name.lexeme) = val;
            return;
        }

        if (enclosing) {
            enclosing->assign(name, val);
            return;
        }

        throw error::RuntimeError{
            name,
            std::format("Undefined variable '{}'.", name.lexeme).c_str()};
    }
}
