#include <format>
#include <string>

#include "core/AstMethods/Print.hpp"

using namespace dxsh;
using namespace core;

declare_method(std::string, PrintRPN, (virtual_<const Expr&>));
declare_method(std::string, PrintInfix, (virtual_<const Expr&>));


define_method(std::string, PrintRPN, (const BinaryExpr& expr)) {
    return std::format("{} {} {}", PrintRPN(*expr.left), PrintRPN(*expr.right), expr.op.GetRepresentation());
}

define_method(std::string, PrintRPN, (const UnaryExpr& expr)) {
    return std::format("{} {}", PrintRPN(*expr.operand), expr.op.GetRepresentation());
}

define_method(std::string, PrintRPN, (const GroupingExpr& expr)) {
    return PrintRPN(*expr.expr);
}

define_method(std::string, PrintRPN, (const LiteralExpr& expr)) {
    return expr.ToString();
}

std::string AstMethods::PrintRPN(const Expr& expr) {
    return ::PrintRPN(expr);
}


define_method(std::string, PrintInfix, (const BinaryExpr& expr)) {
    return std::format("({} {} {})", PrintInfix(*expr.left), expr.op.GetRepresentation(), PrintInfix(*expr.right));
}

define_method(std::string, PrintInfix, (const UnaryExpr& expr)) {
    return std::format("({}{})", expr.op.GetRepresentation(), PrintInfix(*expr.operand));
}

define_method(std::string, PrintInfix, (const GroupingExpr& expr)) {
    return std::format("(group {})", PrintInfix(*expr.expr));
}

define_method(std::string, PrintInfix, (const LiteralExpr& expr)) {
    return expr.ToString();
}

std::string AstMethods::PrintInfix(const Expr& expr) {
    return ::PrintInfix(expr);
}