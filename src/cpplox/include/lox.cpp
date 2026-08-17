#include "lox.hpp"

#include <print>

#include "ast.hpp"

namespace lox {
    bool Lox::hadError = false;

    Lox& Lox::Ins() {
        static Lox instance;
        return instance;
    }

    void Lox::report(size_t line, std::string_view where,
                     std::string_view msg) {
        std::println("[line {}] Error {}: {}", line, where, msg);
        hadError = true;
    }

    void Lox::error(size_t line, std::string_view msg) {
        report(line, "", msg);
    }

    void Lox::error(const lexer::Token& tok, std::string_view msg) {
        if (tok.type == lexer::TokenType::EOF_) {
            report(tok.line, " at end", msg);
        } else {
            report(tok.line, std::format(" at '{}'", tok.lexeme), msg);
        }
    }

    void Lox::run(std::string_view in) {
        lexer::Lexer lexer{in};
        auto tokens = lexer.scanTokens();
        parser::Parser parser{tokens};
        auto expr = parser.parse();

        if (hadError) {
            hadError = false;
            return;
        }

        std::println("{}", ast::astPrinter(expr));
    }

    void Lox::runFile(std::string_view path) {
        auto file = lox::utils::open(path);
        auto ss = std::stringstream{} << file.rdbuf();
        run(ss.str());
    }

    void Lox::runPrompt() {
        replxx::Replxx rx;
        rx.set_max_history_size(1'000);

        std::string buf;

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
            // buf += '\n';
            rx.history_add(buf);
            run(buf);
            buf.clear();
        }
    }

}
