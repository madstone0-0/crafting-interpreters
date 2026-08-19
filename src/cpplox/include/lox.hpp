#pragma once
#include <format>
#include <iostream>
#include <memory>
#include <print>
#include <replxx.hxx>
#include <sstream>
#include <string_view>

#include "ast.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "utils.hpp"

namespace lox {
    struct Lox {
        static bool hadError;

        static bool hadRuntimeError;

        static interpreter::Interpreter interpreter;

        static void error(const lox::lexer::Token&, std::string_view);

        static void error(size_t, std::string_view);

        static void runtimeError(const lox::error::RuntimeError& e);

        static void report(size_t, std::string_view, std::string_view);

        static void run(std::string_view, bool repl = false);

        static int runFile(std::string_view);

        static void runPrompt();

        static Lox& Ins();

       private:
        Lox() = default;
    };
}
