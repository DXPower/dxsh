#pragma once

#include "core/AST.hpp"
#include "core/Error.hpp"

namespace dxsh {
    namespace core {
        class Interpreter;

        // Majority of statements have no captured effect
        // which requires extra processing by the controlling
        // execution context
        enum class StatementEffect {
              None 
            , OpenContext   // Used for opening blocks
            , CloseContext  // Used for break statements
            , InputRequired // Used for input statements
        };

        struct Statement {
            using ExprStore = std::unique_ptr<Expr>;

            int line; 

            Statement(int line) : line(line) { }

            virtual ~Statement() = default;
        };

        struct ExprStatement : Statement {
            ExprStore expr;

            ExprStatement(int line, ExprStore expr) : Statement(line), expr(std::move(expr)) { }
        };

        struct PrintStatement : Statement {
            std::unique_ptr<Expr> expr;

            PrintStatement(int line, ExprStore expr) : Statement(line), expr(std::move(expr)) { }
        };

        struct VarDeclStatement : Statement {
            Token identifier;
            std::unique_ptr<Expr> value;

            VarDeclStatement(int line, const Token& id, ExprStore value)
                : Statement(line), identifier(id), value(std::move(value)) { }
        };
        
        struct BlockStatement : Statement {
            std::vector<std::unique_ptr<Statement>> statements;
            Token open, close;

            BlockStatement(const Token& open, const Token& close, decltype(statements)&& statements)
                : Statement(open.line), statements(std::move(statements)), open(open), close(close) { }
        };

        struct IfStatement : Statement {
            std::unique_ptr<Expr> condition;
            std::unique_ptr<Statement> yesBranch, noBranch;
            Token tokenIf, tokenElse;

            IfStatement(
                  const Token& tokenIf
                , const Token& tokenElse
                , decltype(condition)&& condition
                , decltype(yesBranch)&& yesBranch
                , decltype(noBranch)&&  noBranch
            )   
                : Statement(tokenIf.line)
                , condition(std::move(condition))
                , yesBranch(std::move(yesBranch))
                , noBranch(std::move(noBranch))
                , tokenIf(tokenIf)
                , tokenElse(tokenElse)
            { }
        };

        struct FuncStatement : Statement {
            std::vector<Token> params;
            std::vector<std::unique_ptr<Statement>> statements;
            Token tokenFunc, tokenName;

            FuncStatement(
                  const Token& tokenFunc
                , const Token& tokenName
                , decltype(params)&& params
                , decltype(statements)&& statements
            )   
                : Statement(tokenFunc.line)
                , params(std::move(params))
                , statements(std::move(statements))
                , tokenFunc(tokenFunc)
                , tokenName(tokenName)
            { }
        };


        StatementEffect EvaluateStatement(const Statement& stmt, Interpreter& errors);
    }
}