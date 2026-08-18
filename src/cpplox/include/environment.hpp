#pragma once
#include <format>
#include <string>
#include <unordered_map>

#include "error.hpp"
#include "token.hpp"

namespace lox::environment {
    using EnvMap = std::unordered_map<std::string, lexer::Literal>;

    class Environment {
       private:
        const std::shared_ptr<Environment> enclosing{nullptr};
        EnvMap values;

       public:
        Environment() = default;

        Environment(std::shared_ptr<Environment> enc)
        : enclosing{std::move(enc)} {}

        void define(const std::string&, const lexer::Literal&);

        lexer::Literal get(const lexer::Token&);

        void assign(const lexer::Token&, const lexer::Literal&);
    };

    using EnvPtr = std::shared_ptr<Environment>;
}
