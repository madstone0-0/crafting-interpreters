#include "ast.hpp"

#include <cstddef>

namespace lox::ast {

    std::string astPrinter(const ExprPtr& expr, int depth) {
        if (!expr) return "";

        return std::visit(
            utils::overload{
                [](const Assign& e) {
                    return detail::parenthesize(e.name.lexeme, e.value);
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
                [](const Variable& e) {
                    return detail::parenthesize(e.name.lexeme);
                },
                [](const Logical& e) {
                    return detail::parenthesize(e.op.lexeme, e.left, e.right);
                }},
            *expr);
    }

    std::string astPrinter(const StmtPtr& stmt, int depth) {
        if (!stmt) return "";

        const std::string pad(static_cast<size_t>(depth * 2), ' ');

        return std::visit(
            utils::overload{
                [&](const Block& s) {
                    std::string res = "(block\n";
                    for (const auto& st : s.stmts) {
                        if (!st) continue;
                        res += pad + "  " + astPrinter(st, depth + 1) + "\n";
                    }
                    res += pad + ")";
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
                [](const Break&) { return detail::parenthesize("break"); },
                [](const Expression& s) {
                    return detail::parenthesize("expr", s.expr);
                },
                [](const Print& s) {
                    return detail::parenthesize("print", s.expr);
                },
                [](const Var& s) {
                    return detail::parenthesize(s.name, s.init);
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

}
