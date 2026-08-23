#pragma once
#include <cassert>
#include <format>
#include <print>
#include <string>
#include <unordered_map>

#include "environment_fwd.hpp"
#include "error.hpp"
#include "token.hpp"

namespace lox::environment {
    using EnvMap = std::unordered_map<std::string, lexer::Literal>;
    using Locals = std::vector<lexer::Literal>;

    class Environment : public std::enable_shared_from_this<Environment> {
       private:
        const std::weak_ptr<Environment> enclosing;
        EnvMap values;
        Locals locals;

        std::shared_ptr<Environment> ancestor(int);

       public:
        Environment() = default;

        Environment(const std::shared_ptr<Environment>& enc)
        : enclosing{enc} {}

        void define(const std::string&, const lexer::Literal&);

        lexer::Literal get(const lexer::Token&);

        // lexer::Literal getAt(int, const std::string&);
        lexer::Literal getAt(int, size_t);

        void assign(const lexer::Token&, const lexer::Literal&);

        // void assignAt(int, const lexer::Token&, const lexer::Literal&);
        void assignAt(int, size_t, const lexer::Literal&);
    };
}
