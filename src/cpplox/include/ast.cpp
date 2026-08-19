#include "ast.hpp"

namespace lox::ast {

    std::string astPrinter(const ExprPtr& expr) {
        if (!expr) return "no expression";

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
            },
            *expr);
    }

    std::string astPrinter(const StmtPtr& stmt) {
        return std::visit(utils::overload{
                              [](const Block& s) {
                                  std::string res;
                                  for (const auto& st : s.stmts) {
                                      res += +"\t" + astPrinter(st) + "\n";
                                  }
                                  return std::format("( block\t\n{})", res);
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
