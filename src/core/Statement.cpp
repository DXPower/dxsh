#include <ranges>
#include "magic_enum/magic_enum.hpp"

#include "core/Interpreter.hpp"
#include "core/Statement.hpp"
#include "core/AstMethods/Evaluate.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace dxsh;
using namespace core;

register_classes(
      Statement
    , ExprStatement
    , PrintStatement
    , VarDeclStatement
    , BlockStatement
    , IfStatement
    , FuncStatement
);

declare_method(StatementEffect, EvaluateStatement, (virtual_<const Statement&>, Interpreter*));

define_method(StatementEffect, EvaluateStatement, (const ExprStatement& stmt, Interpreter* interpreter)) {
    AstMethods::Evaluate(*stmt.expr, *interpreter);
    return StatementEffect::None;
}

define_method(StatementEffect, EvaluateStatement, (const VarDeclStatement& stmt, Interpreter* interpreter)) {
    Value res = AstMethods::Evaluate(*stmt.value, *interpreter);
    res = interpreter->GetCurEnvironment().ExtractFromLV(res);

    interpreter->GetCurEnvironment().CreateOrAssignVar(
          stmt.identifier.GetRepresentation()
        , res
        , stmt.line
    );

    return StatementEffect::None;
}

define_method(StatementEffect, EvaluateStatement, (const PrintStatement& stmt, Interpreter* interpreter)) {
    Value res = AstMethods::Evaluate(*stmt.expr, *interpreter);
    res = interpreter->GetCurEnvironment().ExtractFromLV(res);

    interpreter->GiveOutput(res.ToString());
    interpreter->GiveOutput("\n");

    return StatementEffect::None;
}

define_method(StatementEffect, EvaluateStatement, (const BlockStatement& block, Interpreter* interpreter)) {
    interpreter->PushContext(block.statements);
    interpreter->RunInterface();
    
    return StatementEffect::None;
}

define_method(StatementEffect, EvaluateStatement, (const IfStatement& stmt, Interpreter* interpreter)) {
    Value res = AstMethods::Evaluate(*stmt.condition, *interpreter);
    res = interpreter->GetCurEnvironment().ExtractFromLV(res);

    if (auto type = res.GetType(); type != ValueType::Boolean) {
        throw Error{
              .line = stmt.tokenIf.line
            , .message = std::format(
                  "Expected boolean for if condition, got {} instead"
                , magic_enum::enum_name(type)
            )
        };
    }

    if (res.IsTrue()) {
        return core::EvaluateStatement(*stmt.yesBranch, *interpreter);
    } else if (stmt.noBranch != nullptr) {
        return core::EvaluateStatement(*stmt.noBranch, *interpreter);
    }

    return StatementEffect::None;
}

define_method(StatementEffect, EvaluateStatement, (const FuncStatement& func, Interpreter* interpreter)) {
    // interpreter->GiveOutput("\nFunc name: " + std::string(func.tokenName.GetRepresentation()));
    // interpreter->GiveOutput("\nParams: ");

    // for (const Token& param : func.params) {
    //     interpreter->GiveOutput(param.GetRepresentation());
    //     interpreter->GiveOutput(", ");
    // }

    // interpreter->GiveOutput("\nStatements: " + std::to_string(func.statements.size()) + "\n");

    auto funcValue = Function{
          .line = func.tokenFunc.line
        , .name = func.tokenName.GetRepresentation()
        , .params = {}
        , .statements = func.statements
    };

    std::ranges::copy(
          func.params | std::views::transform(&Token::GetRepresentation)
        , std::back_inserter(funcValue.params)
    );

    interpreter->GetCurEnvironment().CreateOrAssignVar(funcValue.name, funcValue, funcValue.line);

    return StatementEffect::None;
}

StatementEffect core::EvaluateStatement(const Statement& stmt, Interpreter& interpreter) {
    try {
        return ::EvaluateStatement(stmt, &interpreter);
    } catch (const Error& e) {
        interpreter.errors.push_back(e);
        return StatementEffect::None;
    }
}
