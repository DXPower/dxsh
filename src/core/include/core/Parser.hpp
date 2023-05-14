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
// expression     → assignment ;
// assignment     → expression "=" assignment
//                | equality                 
// equality       → comparison ( ( "!=" | "==" ) comparison )* ;
// comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
// term           → factor ( ( "-" | "+" ) factor )* ;
// factor         → unary ( ( "/" | "*" ) unary )* ;
// unary          → ( "not" | "-" ) unary
//                | call ;
// call           → primary( "(" arguments? ")" ) ;
// arguments      → expression ( "," expression )* ;
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

            auto Block()       -> StmtStore;
            auto Statement()   -> StmtStore;
            auto PrintStmt()   -> StmtStore;
            auto VarDeclStmt() -> StmtStore;
            auto IfStmt()      -> StmtStore;
            auto FuncStmt()    -> StmtStore;
            auto ExprStmt()    -> StmtStore;
            auto Expression()  -> ExprStore;
            auto Assignment()  -> ExprStore;
            auto Equality()    -> ExprStore;
            auto Comparison()  -> ExprStore;
            auto Term()        -> ExprStore;
            auto Factor()      -> ExprStore;
            auto Unary()       -> ExprStore;
            auto Call()        -> ExprStore;
            auto Arguments()   -> std::vector<ExprStore>;
            auto Primary()     -> ExprStore;

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
                std::size_t delimCount = 0;

                while (true) {
                    auto element = elementGen();

                    if (not element.has_value())
                        break;
                    
                    elements.push_back(std::move(*element));

                    if (Peek().type != delimeter)
                        break;

                    delimCount++;
                    Advance();
                }

                if (elements.size() > 0 && elements.size() == delimCount) {
                    throw Error{
                          .line = Previous().line
                        , .message = std::format(
                              "Unexpected {} in list"
                            , Previous().GetRepresentation()
                        )
                    };
                }

                return elements;
            } 
        };
    }
}