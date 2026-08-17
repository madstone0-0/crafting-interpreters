#include "lexer.hpp"

#include <unordered_map>

namespace lox::lexer {

    const std::unordered_map<std::string, TokenType> Lexer::keywords{
        {"and", TokenType::AND},     {"class", TokenType::CLASS},
        {"else", TokenType::ELSE},   {"false", TokenType::FALSE},
        {"true", TokenType::TRUE},   {"if", TokenType::IF},
        {"fun", TokenType::FUN},     {"for", TokenType::FOR},
        {"nil", TokenType::NIL},     {"or", TokenType::OR},
        {"print", TokenType::PRINT}, {"return", TokenType::RETURN},
        {"super", TokenType::SUPER}, {"this", TokenType::THIS},
        {"var", TokenType::VAR},     {"while", TokenType::WHILE},

    };

    bool Lexer::isDigit(char c) { return c >= '0' && c <= '9'; }

    bool Lexer::isAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool Lexer::isAlphaNumeric(char c) { return isDigit(c) || isAlpha(c); }

    bool Lexer::isAtEnd() { return curr >= source.size(); }

    bool Lexer::match(char c) {
        if (isAtEnd()) return false;
        if (source.at(curr) != c) return false;

        curr++;
        return true;
    }

    char Lexer::peek() {
        if (isAtEnd()) return '\0';
        return source.at(curr);
    }

    char Lexer::peekNext() {
        if (curr + 1 >= source.size()) return '\0';
        return source.at(curr + 1);
    }

    char Lexer::advance() { return source.at(curr++); }

    void Lexer::string() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') line++;
            advance();
        }

        if (isAtEnd()) {
            lox::Lox::error(line, "Unterminated string.");
            return;
        }

        // Consume the terminating "
        advance();

        auto val = source.substr(start + 1, (curr - 1) - (start + 1));
        addToken(TokenType::STRING, val);
    }

    void Lexer::number() {
        while (isDigit(peek())) advance();

        if (peek() == '.' && isDigit(peekNext())) {
            // Consume the .
            advance();

            // Consume the fractional part
            while (isDigit(peek())) advance();
        }

        double num = std::stod(source.substr(start, curr - start));
        addToken(TokenType::NUMBER, num);
    }

    void Lexer::identifer() {
        while (isAlphaNumeric(peek())) advance();

        auto text = source.substr(start, curr - start);
        auto type = keywords.contains(text) ? keywords.at(text)
                                            : TokenType::IDENTIFER;
        addToken(type);
    }

    void Lexer::addToken(TokenType type) { addToken(type, std::monostate{}); }

    void Lexer::addToken(TokenType type, const Literal& literal) {
        auto text = source.substr(start, curr - start);
        tokens.emplace_back(type, text, literal, line);
    }

    void Lexer::scanToken() {
        using enum TokenType;

        auto c = advance();
        switch (c) {
            case ')':
                addToken(RIGHT_PAREN);
                break;
            case '(':
                addToken(LEFT_PAREN);
                break;
            case '}':
                addToken(RIGHT_BRACE);
                break;
            case '{':
                addToken(LEFT_BRACE);
                break;
            case '.':
                addToken(DOT);
                break;
            case ',':
                addToken(COMMA);
                break;
            case '+':
                addToken(PLUS);
                break;
            case '-':
                addToken(MINUS);
                break;
            case ';':
                addToken(SEMICOLON);
                break;
            case '*':
                addToken(STAR);
                break;
            case '?':
                addToken(QUESTION_MARK);
                break;
            case ':':
                addToken(COLON);
                break;

            case '!':
                addToken(match('=') ? BANG_EQUAL : BANG);
                break;

            case '=':
                addToken(match('=') ? EQUAL_EQUAL : EQUAL);
                break;

            case '<':
                addToken(match('=') ? LESS_EQUAL : LESS);
                break;

            case '>':
                addToken(match('=') ? GREATER_EQUAL : GREATER);
                break;

            case '/': {
                if (match('/')) {
                    // A comment goes until the end of the line
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else if (match('*')) {
                    while (!isAtEnd()) {
                        if (peek() == '*' && peekNext() == '/') {
                            advance();
                            advance();
                            break;
                        }
                        if (peek() == '\n') line++;
                        advance();
                    }
                    if (isAtEnd())
                        lox::Lox::error(line, "Unterminated comment.");
                } else {
                    addToken(SLASH);
                }
            } break;

            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace
                break;

            case '\n':
                line++;
                break;

            case '"':
                string();
                break;

            default:
                if (isDigit(c)) {
                    number();
                } else if (isAlpha(c)) {
                    identifer();
                } else {
                    lox::Lox::error(line, "Unexpected character.");
                }
                break;
        }
    }

    Tokens Lexer::scanTokens() {
        using enum TokenType;
        while (!isAtEnd()) {
            // At the beginning of the next lexeme
            start = curr;
            scanToken();
        }

        tokens.emplace_back(EOF_, "", std::monostate{}, line);
        return tokens;
    }
}
