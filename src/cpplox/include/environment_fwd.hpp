#pragma once

#include <memory>

namespace lox::environment {
    class Environment;
    using EnvPtr = std::shared_ptr<Environment>;
    using EnvWeakPtr = std::weak_ptr<Environment>;
}
