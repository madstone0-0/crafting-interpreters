#include "literal.hpp"

#include "environment.hpp"

namespace lox::lexer {

    lexer::Literal Callable::call(const environment::EnvPtr& env,
                                  const Literals& args) const {
        return f(env, args);
    }
}
