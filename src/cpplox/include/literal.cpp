#include "literal.hpp"

#include "environment.hpp"
#include "token.hpp"

namespace lox::lexer {

    lexer::Literal Callable::call(const Literals& args) const {
        return f(std::make_shared<environment::Environment>(closure), args);
    }

    Callable Callable::bind(InstanceLiteral ins) {
        auto env = std::make_shared<environment::Environment>(closure);
        env->define("this", std::move(ins));
        return Callable{
            .name = name,
            .arity_ = arity_,
            .closure = env,
            .f = f,
            .isInit = isInit,
        };
    }

    lexer::Literal Class::call(const Literals& args) const {
        auto ins = std::make_shared<Instance>(shared_from_this());
        auto init = findMethod("init");
        if (init.has_value()) init->bind(ins).call(args);

        return ins;
    }

    size_t Class::arity() const {
        auto init = findMethod("init");
        if (!init.has_value()) return 0;
        return init->arity();
    }

    std::optional<lexer::CallableLiteral> Class::findMethod(
        const std::string& name) const {
        if (!methods.contains(name)) return {};
        return methods.at(name);
    }

    lexer::Literal Instance::access(const lexer::Token& name) {
        // NOTE: Fields shadow methods
        if (fields.contains(name.lexeme)) return fields.at(name.lexeme);

        auto method = c->findMethod(name.lexeme);
        if (method.has_value()) return method->bind(shared_from_this());

        throw error::RuntimeError{
            name, std::format("Undefined property {}.", name.lexeme).c_str()};
    }

    void Instance::set(const lexer::Token& name, const lexer::Literal& val) {
        fields.emplace(name.lexeme, val);
    }
}
