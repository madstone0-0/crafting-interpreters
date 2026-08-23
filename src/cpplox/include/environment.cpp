#include "environment.hpp"

namespace lox::environment {

    EnvPtr Environment::ancestor(int dist) {
        auto curr = shared_from_this();

        for (int i = 0; i < dist; ++i) {
            if (!curr->enclosing.expired()) {
                curr = curr->enclosing.lock();
            } else {
                throw std::runtime_error("No enclosing environment found.");
            }
        }

        return curr;
    }

    void Environment::define(const std::string& name,
                             const lexer::Literal& val) {
        locals.emplace_back(val);
        values.insert({name, val});
    }

    lexer::Literal Environment::get(const lexer::Token& name) {
        if (values.contains(name.lexeme)) return values.at(name.lexeme);

        if (auto encPtr = enclosing.lock()) return encPtr->get(name);

        throw error::RuntimeError{
            name,
            std::format("Undefined variable '{}'.", name.lexeme).c_str()};
    }

    lexer::Literal Environment::getAt(int dist, size_t idx) {
        return ancestor(dist)->locals.at(idx);
    }

    void Environment::assign(const lexer::Token& name,
                             const lexer::Literal& val) {
        if (values.contains(name.lexeme)) {
            values.at(name.lexeme) = val;
            return;
        }

        if (auto encPtr = enclosing.lock()) {
            encPtr->assign(name, val);
            return;
        }

        throw error::RuntimeError{
            name,
            std::format("Undefined variable '{}'.", name.lexeme).c_str()};
    }

    void Environment::assignAt(int dist, size_t idx,
                               const lexer::Literal& value) {
        ancestor(dist)->locals.at(idx) = value;
    }
}
