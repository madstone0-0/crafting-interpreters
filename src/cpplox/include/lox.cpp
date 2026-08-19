#include "lox.hpp"

#include <print>

#include "ast.hpp"

namespace {
    bool isBalanced(const std::string& src) {
        using enum lox::lexer::TokenType;

        auto savedError = lox::Lox::hadError;

        lox::lexer::Lexer lex{src};
        const auto& tokens = lex.scanTokens();

        lox::Lox::hadError = savedError;

        int depth = 0;
        for (const auto& tok : tokens) {
            switch (tok.type) {
                case LEFT_PAREN:
                case LEFT_BRACE:
                    depth++;
                    break;
                case RIGHT_PAREN:
                case RIGHT_BRACE:
                    depth--;
                    break;
                default:
                    break;
            }
        }
        return depth <= 0;
    }
}

namespace lox {
    bool Lox::hadError = false;
    bool Lox::hadRuntimeError = false;
    interpreter::Interpreter Lox::interpreter{};

    Lox& Lox::Ins() {
        static Lox instance;
        return instance;
    }

    void Lox::report(size_t line, std::string_view where,
                     std::string_view msg) {
        std::println(std::cerr, "[line {}] Error {}: {}", line, where, msg);
        hadError = true;
    }

    void Lox::error(size_t line, std::string_view msg) {
        report(line, "", msg);
    }

    void Lox::error(const lexer::Token& tok, std::string_view msg) {
        if (tok.type == lexer::TokenType::EOF_) {
            report(tok.line, " at end", msg);
        } else {
            report(tok.line, std::format("at '{}'", tok.lexeme), msg);
        }
    }

    void Lox::runtimeError(const lox::error::RuntimeError& e) {
        std::println(std::cerr, "{}\n[line {}]", e.what(), e.token.line);
        hadRuntimeError = true;
    }

    void Lox::run(std::string_view in, bool repl) {
        lexer::Lexer lexer{in};
        auto tokens = lexer.scanTokens();
        parser::Parser parser{tokens};
        auto stmts = parser.parse();

        if (hadError) return;

#ifdef DEBUG
        for (const auto& tok : tokens) std::println("{}", std::string{tok});
#endif
        interpreter.interpret(std::move(stmts), repl);
    }

    int Lox::runFile(std::string_view path) {
        auto file = lox::utils::open(path);
        auto ss = std::stringstream{} << file.rdbuf();
        run(ss.str());

        if (hadError) return 65;
        if (hadRuntimeError) return 70;
        return 0;
    }

    void Lox::runPrompt() {
        replxx::Replxx rx;
        rx.set_max_history_size(1'000);

        std::string buf;

        std::println(R"(CPPLox v0.1.0
Type ".exit" to exit the REPL)");
        while (true) {
            const char* prompt = buf.empty() ? "> " : "...";
            const char* input = rx.input(prompt);

            if (input == nullptr) {
                if (!buf.empty()) {
                    std::println();
                    buf.clear();
                    continue;
                }
                break;
            }

            std::string line{input};
            if (buf.empty() && line == ".exit") break;

            buf += line;

            if (!isBalanced(buf)) {
                continue;
            }

            rx.history_add(buf);
            run(buf, true);
            buf.clear();
            hadError = false;
            hadRuntimeError = false;
        }
    }

}
