#pragma once

#include <concepts>
#include <span>
#include "AST.hpp"
#include "Error.hpp"
#include "Statement.hpp"


// program        → (block)* EOF
// block          → "{" block* "}"
//                | statement
// statement      → (exprstmt  | printstmt | vardeclstmt | ifstmt | funcstmt)
// printstmt      → "print" exprstmt
// vardeclstmt    → "var" IDENTIFIER "=" expression ";"
// ifstmt         → "if" "(" expression ")" block
//                  ( "else" block )?
// funcstmt       → "func" IDENTIFIER "(" IDENTIFIER* ")" "{" block* "}"
// exprstmt       → expression ";"
// expression     → assignment 
// assignment     → expression "=" assignment
//                | equality                 
// equality       → comparison ( ( "!=" | "==" ) comparison )* ;
// comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
// term           → factor ( ( "-" | "+" ) factor )* ;
// factor         → unary ( ( "/" | "*" ) unary )* ;
// unary          → ( "not" | "-" ) unary
//                | primary ;
// primary        → INTEGER | DECIMAL | STRING | "true" | "false" | "null" | IDENTIFIER
//                | "(" expression ")" ;

namespace dxsh {
    namespace core {
        namespace detail {
            template<typename T>
            struct IsOptional : std::false_type { };

            template<typename T>
            struct IsOptional<std::optional<T>> : std::true_type { };

            template<typename T>
            concept IsOptional_c = IsOptional<T>::value;
        }

        class Parser {
            using ExprStore = std::unique_ptr<Expr>;
            using StmtStore = std::unique_ptr<Statement>;

            ErrorContext* errors;

            std::span<const Token> tokens;
            std::size_t curPos{};
            
            public:
            Parser(ErrorContext& errors) : errors(&errors) { }

            auto Parse(std::span<const Token> tokens) -> std::vector<StmtStore>;

            StmtStore Block();
            StmtStore Statement();
            StmtStore PrintStmt();
            StmtStore VarDeclStmt();
            StmtStore IfStmt();
            StmtStore FuncStmt();
            StmtStore ExprStmt();
            ExprStore Expression();
            ExprStore Assignment();
            ExprStore Equality();
            ExprStore Comparison();
            ExprStore Term();
            ExprStore Factor();
            ExprStore Unary();
            ExprStore Primary();

            const Token& Advance();
            const Token& TryConsume(TokenType type, std::string_view error);

            const Token& Previous() const;
            const Token& Peek() const;
            bool Check(TokenType type) const;
            bool IsAtEnd() const;

            bool MatchConsume(std::same_as<TokenType> auto... types) {
                for (TokenType type : { types... }) {
                    if (Check(type)) {
                        Advance();
                        return true;
                    }
                }

                return false;
            }

            private:
            void Synchronize();

            template<auto NestedExpression>
            ExprStore ParseBinaryExpression(std::same_as<TokenType> auto... types) {
                auto expr = (this->*NestedExpression)();

                while (MatchConsume(types...)) {
                    const auto& op = Previous();

                    expr = std::make_unique<BinaryExpr>(
                          std::move(expr)             // Left branch
                        , (this->*NestedExpression)() // Right branch
                        , op                          // Operator
                    );
                }

                return expr;
            }

            auto ParseList(TokenType delimeter, const auto& elementGen)
            requires requires(const decltype(elementGen)& gen) {
                { elementGen() } -> detail::IsOptional_c;
            } {
                using Element_t = typename std::invoke_result_t<decltype(elementGen)>::value_type;
                std::vector<Element_t> elements;

                while (true) {
                    auto element = elementGen();

                    if (not element.has_value())
                        break;
                    
                    elements.push_back(std::move(*element));

                    if (Peek().type != delimeter)
                        break;

                    Advance();
                }

                return elements;
            } 
        };
    }
}