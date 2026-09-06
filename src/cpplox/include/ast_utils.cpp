#include "ast_utils.hpp"

#include <cstddef>
#include <format>

namespace lox::ast {
    std::string astPrinter(const lexer::Tokens& tokens, int depth) {
        std::string res;

        for (size_t i{}; i < tokens.size(); ++i) {
            if (i != 0) {
                res += ' ';
            }

            res += tokens[i].lexeme;
        }

        return res;
    }

    std::string astPrinter(const std::vector<StmtPtr>& stmts, int depth) {
        std::string res;

        for (const auto& stmt : stmts) {
            if (!stmt) continue;

            if (!res.empty()) res += ' ';

            res += astPrinter(stmt, depth);
        }

        return res;
    }

    std::string astPrinter(const std::vector<ExprPtr>& exprs, int depth) {
        std::string res;

        for (const auto& e : exprs) {
            if (!e) continue;

            if (!res.empty()) res += ' ';

            res += astPrinter(e, depth);
        }

        return res;
    }

    std::string astPrinter(const lexer::Tokens& tokens,
                           const std::vector<StmtPtr>& stmts, int depth) {
        std::string res = astPrinter(tokens, depth);
        const auto body = astPrinter(stmts, depth);

        if (!res.empty() && !body.empty()) {
            res += ' ';
        }

        res += body;
        return res;
    }

    std::string astPrinter(const ExprPtr& expr, int depth) {
        if (!expr) return "";

        return std::visit(
            utils::overload{
                [](const Assign& e) {
                    return detail::parenthesize(e.name.lexeme, e.val);
                },
                [](const Binary& e) {
                    return detail::parenthesize(e.op.lexeme, e.left, e.right);
                },
                [](const Ternary& e) {
                    return detail::parenthesize(e.op1.lexeme + e.op2.lexeme,
                                                e.left, e.mid, e.right);
                },
                [](const Grouping& e) {
                    return detail::parenthesize("group", e.expr);
                },
                [](const Literal& e) {
                    return std::visit(LiteralPrinter, e.val);
                },
                [](const Unary& e) {
                    return detail::parenthesize(e.op.lexeme, e.right);
                },
                [](const Variable& e) { return e.name.lexeme; },
                [](const Logical& e) {
                    return detail::parenthesize(e.op.lexeme, e.left, e.right);
                },
                [](const Call& e) {
                    return detail::parenthesize("call", e.callee, e.args);
                },
                [](const Get& e) {
                    return detail::parenthesize(
                        std::format("get {}", e.name.lexeme), e.obj);
                },
                [](const Set& e) {
                    return detail::parenthesize(
                        std::format("set {}", e.name.lexeme), e.obj, e.val);
                },
                [](const This& e) {
                    return detail::parenthesize(e.keyword.lexeme);
                },
                [](const Lambda& e) {
                    return detail::parenthesize("lambda", e.params, e.body);
                }},
            *expr);
    }

    std::string astPrinter(const StmtPtr& stmt, int depth) {
        if (!stmt) return "";

        const std::string pad(static_cast<size_t>(depth * 2), ' ');

        return std::visit(
            utils::overload{
                [depth](const Block& s) {
                    std::string res = "(block\n";
                    for (const auto& st : s.stmts) {
                        if (!st) {
                            continue;
                        }
                        const std::string child = astPrinter(st, depth + 1);
                        res += std::string(
                            static_cast<std::size_t>((depth + 1) * 2), ' ');
                        res += child;
                        res += '\n';
                    }

                    res += std::string(static_cast<std::size_t>(depth * 2),
                                       ' ');
                    res += ')';

                    return res;
                },
                [](const If& s) {
                    return detail::parenthesize("if", s.condition, s.thenBr,
                                                s.elseBr);
                },
                [](const While& s) {
                    return detail::parenthesize("while", s.condition, s.body);
                },
                [](const For& s) {
                    return detail::parenthesize("for", s.init, s.condition,
                                                s.inc);
                },
                [](const Return& s) {
                    return detail::parenthesize(s.keyword.lexeme, s.val);
                },
                [](const Break& s) {
                    return detail::parenthesize(s.keyword.lexeme);
                },
                [](const Expression& s) {
                    return detail::parenthesize("expr", s.expr);
                },
                [](const Print& s) {
                    return detail::parenthesize("print", s.expr);
                },
                [](const Var& s) {
                    return detail::parenthesize(s.name, s.init);
                },
                [](const Function& s) {
                    return detail::parenthesize(s.name.lexeme, s.params,
                                                s.body);
                },
                [](const Class& s) {
                    return detail::parenthesize(
                        std::format("class {}", s.name.lexeme));
                },
            },
            *stmt);
    }

    // std::string rpnAstPrinter(const ExprPtr& expr) {
    //     if (!expr) return "no expression";
    //
    //     return std::visit(
    //         utils::overload{
    //             [](const Assign& e) {
    //                 return rpnAstPrinter(e.value) + " " +
    //                        std::format("[{}]", e.name.lexeme);
    //             },
    //             [](const Binary& e) {
    //                 return rpnAstPrinter(e.left) + " " +
    //                        rpnAstPrinter(e.right) + " " + e.op.lexeme;
    //             },
    //             [](const Ternary& e) {
    //                 return rpnAstPrinter(e.left) + " " +
    //                 rpnAstPrinter(e.mid) +
    //                        " " + rpnAstPrinter(e.right) + " " + e.op1.lexeme
    //                        + e.op2.lexeme;
    //             },
    //             [](const Unary& e) {
    //                 return rpnAstPrinter(e.right) + " " + e.op.lexeme;
    //             },
    //             [](const Grouping& e) { return rpnAstPrinter(e.expr); },
    //             [](const Literal& e) {
    //                 return std::visit(LiteralPrinter, e.val);
    //             },
    //             [](const Variable& e) { return e.name.lexeme; },
    //         },
    //         *expr);
    // }

    ast::Function& parseMethod(const StmtPtr& meth) {
        if (auto* f = std::get_if<ast::Function>(meth.get())) {
            return *f;
        }

        throw std::runtime_error{
            "A non-function node was passed as a method. This should not be "
            "possible!"};
    }
}
