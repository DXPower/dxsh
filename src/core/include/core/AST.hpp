#pragma once

#include <memory>
#include <any>
#include <variant>

#include <yorel/yomm2/keywords.hpp>

#include "Tokens.hpp"
#include "Value.hpp"

/*
    Grammar:
    expression → literal
               | unary
               | binary
               | grouping

    literal  → NUMBER | STRING | "true" | "false" | "null"
    grouping → "(" expression ")"
    unary    → ( "-" | "not" ) expression
    binary   → expression operator expression
    operator → "and" | "or"
             | "=="  | "!=" | "<" | "<=" | ">" | ">=" | "**" 
             | "="   | "+"  | "-" | "*"  | "/"
*/

namespace dxsh {
    namespace core {
        struct Expr {
            virtual ~Expr() = default;
        };

        struct BinaryExpr : Expr {
            std::unique_ptr<Expr> left, right;
            Token op;

            BinaryExpr() = default;
            BinaryExpr(decltype(left)&& left, decltype(right)&& right, Token op)
                : left(std::move(left)), right(std::move(right)), op(std::move(op))
            { }
        };

        struct UnaryExpr : Expr {
            std::unique_ptr<Expr> operand;
            Token op;

            UnaryExpr() = default;
            UnaryExpr(decltype(operand)&& operand, Token op)
                : operand(std::move(operand)), op(std::move(op))
            { }
        };

        struct GroupingExpr : Expr {
            std::unique_ptr<Expr> expr;

            GroupingExpr() = default;
            GroupingExpr(decltype(expr)&& expr)
                : expr(std::move(expr))
            { }
        };

        struct LiteralExpr : Expr {
            Value value;
            Token token;

            LiteralExpr(Token token) : token(std::move(token)) { };
            LiteralExpr(Value value, const Token& token)
                : value(std::move(value)), token(token)
            { }

            ValueType GetType() const { return value.GetType(); };
            std::string ToString() const { return value.ToString(); };
        };

        struct AssignmentExpr : Expr {
            std::unique_ptr<Expr> target;
            std::unique_ptr<Expr> value;
            Token equal;

            AssignmentExpr(decltype(target)&& target, decltype(value)&& value, const Token& equal)
                : target(std::move(target))
                , value(std::move(value))
                , equal(equal)
            { }
        };

        struct CallExpr : Expr {
            std::unique_ptr<Expr> function;
            std::vector<std::unique_ptr<Expr>> args;
            Token parenL;

            CallExpr(decltype(function)&& func, decltype(args)&& args, const Token& parenL)
                : function(std::move(func))
                , args(std::move(args))
                , parenL(parenL)
            { }
        };

        register_classes(
              Expr
            , BinaryExpr
            , UnaryExpr
            , GroupingExpr
            , LiteralExpr
            , AssignmentExpr
            , CallExpr
        );
    }
}