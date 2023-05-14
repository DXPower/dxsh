#include "core/Parser.hpp"
#include "core/AST.hpp"
#include "core/Error.hpp"
#include "core/Statement.hpp"
#include "core/Value.hpp"
#include <memory>

using namespace dxsh;
using namespace core;

auto Parser::Parse(std::span<const Token> tokens) -> std::vector<StmtStore> {
    this->tokens = tokens;
    curPos = 0;

    std::vector<StmtStore> statements;

    while (not IsAtEnd()) {
        try {
            statements.push_back(Block());
        } catch (const Error& e) { 
            errors->push_back(e);
            Synchronize();
        }
    }

    return statements;
}

auto Parser::Block() -> StmtStore {
    using enum TokenType;

    if (MatchConsume(BraceL)) {
        const Token& open = Previous();
        std::vector<StmtStore> statements;

        while (not IsAtEnd()) {
            if (MatchConsume(BraceR)) {
                const Token& close = Previous();

                return std::make_unique<BlockStatement>(open, close, std::move(statements));
            }

            statements.push_back(Block());
        }

        throw Error {
              .line = Peek().line
            , .message = std::format(
                  "Unclosed block at EOF (opened at line {})"
                , open.line
            )
        };
    } else {
        return Statement();
    }
}

auto Parser::Statement() -> StmtStore {
    using enum TokenType;

    ExprStore expr;
    StmtStore stmt;

    if (MatchConsume(Print)) {
        stmt = PrintStmt();
    } else if (MatchConsume(Var)) {
        stmt = VarDeclStmt();
    } else if (MatchConsume(If)) {
        return IfStmt(); // Return here since if doesn't need semicolon
    } else if (MatchConsume(Function)) {
        return FuncStmt(); // Return here since function doesn't need semicolon
    } else if (MatchConsume(Return)) {
        stmt = ReturnStmt();
    } else {
        stmt = ExprStmt();
    }

    TryConsume(TokenType::Semicolon, std::format(
          "Expected semicolon after expression, got {}"
        , Peek().GetRepresentation()
    ));

    return stmt;
}

auto Parser::PrintStmt() -> StmtStore {
    int line = Previous().line;

    return std::make_unique<PrintStatement>(
          line
        , Expression()
    );
}

auto Parser::VarDeclStmt() -> StmtStore {
    using enum TokenType;

    int line = Previous().line;

    const auto& ident = TryConsume(Identifier, std::format(
            "Expected identifier after 'var', got {}"
        , Peek().GetRepresentation()
    ));

    TryConsume(Equal, std::format(
            "Expected '=' after var declaration identifier, got {}"
        , Peek().GetRepresentation()
    ));

    return std::make_unique<VarDeclStatement>(
          line
        , ident
        , Expression()
    );
}

auto Parser::IfStmt() -> StmtStore {
    using enum TokenType;

    const Token& ifToken = Previous();
    TryConsume(ParenL, "Expected '(' to start if statement's condition");
    auto condition = Expression();
    TryConsume(ParenR, "Expected ')' to close if statement's condition");
    auto yesBranch = Block();

    if (MatchConsume(Else)) {
        const Token& elseToken = Previous();
        auto noBranch = Block();

        return std::make_unique<IfStatement>(
              ifToken, elseToken
            , std::move(condition)
            , std::move(yesBranch), std::move(noBranch)
        );
    } else {
        return std::make_unique<IfStatement>(
              ifToken, Token{}
            , std::move(condition)
            , std::move(yesBranch), nullptr
        );
    }
}

auto Parser::FuncStmt() -> StmtStore {
    using enum TokenType;

    const Token& funcToken = Previous();
    const Token& funcName = TryConsume(
          Identifier
        , std::format(
              "Expected identifier for function name, got '{}' instead"
            , magic_enum::enum_name(Peek().type)
        )
    );

    TryConsume(
          ParenL
        , std::format(
              "Expected '(' after function name, got '{}' instead"
            , Peek().GetRepresentation()
        )
    );

    std::vector<Token> params = ParseList(Comma, [this]() -> std::optional<Token> {
        if (Peek().type == Identifier)
            return Advance();
        else
            return std::nullopt;
    });

    TryConsume(
          ParenR
        , std::format(
              "Expected ')' after function parameter list, got '{}' instead"
            , Peek().GetRepresentation()
        )
    );

    TryConsume(
          BraceL
        , std::format(
              "Expected '{{' after function parameter list, got '{}' instead"
            , Peek().GetRepresentation()
        )
    );

    std::vector<StmtStore> statements;

    while (not IsAtEnd() and Peek().type != BraceR) {
        statements.push_back(Block());
    }

    // Insert a void return statement (return;) if there isn't a
    // return statement at the end of the function block
    if (statements.size() == 0 || !dynamic_cast<ReturnStatement*>(statements.back().get())) {
        statements.push_back(std::make_unique<ReturnStatement>(Peek().line, nullptr));
    }

    TryConsume(
          BraceR
        , std::format(
              "Expected '}}' after function definition, got '{}' instead"
            , Peek().GetRepresentation()
        )
    );

    return std::make_unique<FuncStatement>(
          funcToken
        , funcName
        , std::move(params)
        , std::move(statements)
    );
}

auto Parser::ReturnStmt() -> StmtStore {
    int line = Previous().line;

    return std::make_unique<ReturnStatement>(
          line
        , Peek().type != TokenType::Semicolon ? Expression() : nullptr
    );
}

auto Parser::ExprStmt() -> StmtStore {
    int line = Peek().line;

    return std::make_unique<ExprStatement>(
          line
        , Expression()
    );
}

auto Parser::Expression() -> ExprStore {
    return Assignment();
} 

auto Parser::Assignment() -> ExprStore {
    auto expr = Equality();

    if (MatchConsume(TokenType::Equal)) {
        const auto& equal = Previous();
        
        return std::make_unique<AssignmentExpr>(std::move(expr), Assignment(), equal);
    }

    return expr;
} 


auto Parser::Equality() -> ExprStore {
    return ParseBinaryExpression<&Parser::Comparison>(
          TokenType::EqualEqual
        , TokenType::BangEqual
    );
}

auto Parser::Comparison() -> ExprStore {
   return ParseBinaryExpression<&Parser::Term>(
          TokenType::Greater
        , TokenType::GreaterEqual
        , TokenType::Less
        , TokenType::LessEqual
    );
}

auto Parser::Term() -> ExprStore {
    return ParseBinaryExpression<&Parser::Factor>(
          TokenType::Plus
        , TokenType::Minus
    );
}

auto Parser::Factor() -> ExprStore {
    return ParseBinaryExpression<&Parser::Unary>(
          TokenType::Star
        , TokenType::Slash
    );
}

auto Parser::Unary() -> ExprStore {
    if (MatchConsume(TokenType::Not, TokenType::Minus)) {
        const auto& op = Previous();

        return std::make_unique<UnaryExpr>(
              Unary()    // Operand
            , op
        );
    }

    return Call();
}

auto Parser::Call() -> ExprStore {
    auto expr = Primary();

    // Keep parsing function calls to support things like a()()()
    while (true) {
        if (not MatchConsume(TokenType::ParenL)) {
            break;
        }

        const auto& parenL = this->Previous();
        auto args = Arguments();

        TryConsume(TokenType::ParenR, std::format("Expected ) after function call arguments, got '{}' instead", Peek().GetRepresentation()));
    
        expr = std::make_unique<CallExpr>(std::move(expr), std::move(args), parenL);
    }

    return expr;
}

auto Parser::Arguments() -> std::vector<ExprStore> {
    using enum TokenType;

    std::vector<ExprStore> args = ParseList(Comma, [this]() -> std::optional<ExprStore> {
        if (Peek().type != ParenR && Peek().type != Comma)
            return Expression();
        else
            return std::nullopt;
    });
    
    return args;
}

auto Parser::Primary() -> ExprStore {
    const Token& curToken = Peek();

    if (MatchConsume(TokenType::Null))  return std::make_unique<LiteralExpr>(curToken);
    if (MatchConsume(TokenType::True))  return std::make_unique<LiteralExpr>(true, curToken);
    if (MatchConsume(TokenType::False)) return std::make_unique<LiteralExpr>(false, curToken);

    if (MatchConsume(TokenType::Integer)) 
        return std::make_unique<LiteralExpr>(std::get<int>(curToken.literal), curToken);

    if (MatchConsume(TokenType::Decimal)) 
        return std::make_unique<LiteralExpr>(std::get<float>(curToken.literal), curToken);

    if (MatchConsume(TokenType::String))
        return std::make_unique<LiteralExpr>(std::get<std::string>(curToken.literal), curToken);

    if (MatchConsume(TokenType::Identifier))
        return std::make_unique<LiteralExpr>(
              Lvalue{curToken.line, curToken.lexeme}
            , curToken
        );

    if (MatchConsume(TokenType::ParenL)) {
        auto expr = Expression();
        TryConsume(TokenType::ParenR, "Expected ')' after parenthetical expression");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    throw Error{
          .line = Peek().line
        , .message = std::format("Expected primary expression, not token '{}'", Peek().GetRepresentation())
    };
}

void Parser::Synchronize() {
    using enum TokenType;

    while (not IsAtEnd()) {
        if (Advance().type == Semicolon) return;

        switch (Peek().type) {
            case Var:
            case Print:
            case If:
            case For:
            case While:
            case Function:
            case Return:
                return;
            default:
                continue;
        }
    }
}

const Token& Parser::Advance() {
    if (not IsAtEnd())
        curPos++;

    return Previous();
}

const Token& Parser::TryConsume(TokenType type, std::string_view error) {
    if (Check(type)) return Advance();

    throw Error{
          .line = Peek().line
        , .message = std::string(error)
    };
}

const Token& Parser::Previous() const {
    return tokens[curPos - 1];
}

const Token& Parser::Peek() const {
    return tokens[curPos];
}

bool Parser::Check(TokenType type) const {
    if (IsAtEnd())
        return false;

    return Peek().type == type;
}

bool Parser::IsAtEnd() const {
    return Peek().type == TokenType::Eof;
}