#include <stdexcept>
#include <utility>
#include "core/AstMethods/Evaluate.hpp"
#include "core/Defer.hpp"
#include "core/Environment.hpp"
#include "core/Error.hpp"
#include "core/Value.hpp"
#include "magic_enum/magic_enum.hpp"

using namespace dxsh;
using namespace core;

enum class UnaryConversionTarget { Numeric, Boolean };

struct BinaryConversionResult {
    Value left, right;
};

static std::optional<BinaryConversionResult> NumericConversion(const Value& left, const Value& right) {
    using Res = BinaryConversionResult;
    using enum ValueType;

    ValueType typeL = left.GetType();
    ValueType typeR = right.GetType();

    // Only support arithmetic operations on int, float
    if (not left.IsArithmetic() || not right.IsArithmetic())
        return std::nullopt;

    // No conversion necessary
    if (typeL == typeR)
        return Res{ left, right };

    // Two options remaining - either left is an int, or right is an int, and it needs
    // to be converted to a float
    if (typeL == Integer) {
        return Res{
              .left = static_cast<float>(left.GetAs<int>())
            , .right = right
        };
    }

    if (typeR == Integer) {
        return Res{
              .left = left
            , .right = static_cast<float>(right.GetAs<int>())
        };
    }
    
    return std::nullopt; // Should not be reachable
}

template<UnaryConversionTarget Target>
static std::optional<Value> UnaryConversion(const Value& value) {
    if constexpr (Target == UnaryConversionTarget::Numeric) {
        if (value.GetType() != ValueType::Integer && value.GetType() != ValueType::Decimal) {
            return value;
        }
    }
}

// Assumes left and right are the same type
// Only operates on Arithmetic or Comparison operator classes
// Accepts integer, floating, and strings
static Value EvaluateBinaryExpr(const Value& left, const Value& right, const Token& op) {
    using enum TokenType;

    auto eval = [&]<typename T>() -> std::variant<T, bool> {
        const T& l = left.GetAs<T>();
        const T& r = right.GetAs<T>();
    
        // Special case the operations that aren't valid for strings
        if constexpr (not std::same_as<T, std::string>) {
            switch (op.type) {
                // Arithmetic
                case Minus: return l - r;
                case Star:  return l * r;
                case Slash: return l / r;
                case StarStar: // Exponent
                    if constexpr (std::same_as<T, int>) {
                        if (r == 0) return 1; // 0^0 == 1
                        if (r == 1) return l;
                        if (l == 0) return 0;

                        int val = l;

                        for (int i = 1; i < r; i++) {
                            val *= l;
                        }

                        return l;
                    } else {
                        return std::powf(l, r);
                    }
                default: ; // Fallthrough to next switch statement
            }
        }
        
        switch (op.type) {
            case Plus:  return l + r;
            // Comparison
            case Greater:      return l > r;
            case GreaterEqual: return l >= r;
            case Less:         return l < r;
            case LessEqual:    return l <= r;
            default:
                throw Error{
                      .line = op.line
                    , .message = std::format(
                          "Invalid binary operator {} for types {} and {}"
                        , op.GetRepresentation()
                        , magic_enum::enum_name(left.GetType())
                        , magic_enum::enum_name(right.GetType())
                    )
                };
        }
    };

    if (left.GetType() == ValueType::Integer) {
        if (GetTokenClass(op.type) == TokenClass::Arithmetic)
            return std::get<int>(eval.operator()<int>());
        else
            return std::get<bool>(eval.operator()<int>());
    } else if (left.GetType() == ValueType::Decimal) {
        if (GetTokenClass(op.type) == TokenClass::Arithmetic)
            return std::get<float>(eval.operator()<float>());
        else
            return std::get<bool>(eval.operator()<float>());
    } else if (left.GetType() == ValueType::String) {
        if (GetTokenClass(op.type) == TokenClass::Arithmetic)
            return std::get<std::string>(eval.operator()<std::string>());
        else
            return std::get<bool>(eval.operator()<std::string>());
    }

    throw std::runtime_error(
        std::format(
              "Unexpectedly reached end of EvaluateBinaryExpr. {} {} {}"
            , magic_enum::enum_name(left.GetType())
            , op.GetRepresentation()
            , magic_enum::enum_name(right.GetType())
        )
    );
}

static bool EvaluateEquality(const Value& left, const Value& right, const Token& op) {
    using enum ValueType;

    ValueType leftT = left.GetType();
    ValueType rightT = right.GetType();

    // IILE to deal with == vs != in a concise way
    bool result = [&]() {
        if (leftT == rightT) {
            switch (leftT) {
                case Integer:    return left.GetAs<int>() == right.GetAs<int>();
                case Decimal:    return left.GetAs<float>() == right.GetAs<float>();
                case String:     return left.GetAs<std::string>() == right.GetAs<std::string>();
                case Boolean:    return left.GetAs<bool>() == right.GetAs<bool>();
                case Null:       return true;
                case Lvalue:     throw std::runtime_error("Unextracted lvalue in equality");
            }
        }

        // Handle the case of two different types being compared
        if (leftT == Null || rightT == Null)
            return false;

        // Perform an implicit int->float conversion
        if (left.IsArithmetic() && right.IsArithmetic()) {
            auto res = NumericConversion(left, right);

            return res->left.GetAs<float>() == res->right.GetAs<float>();
        }

        // Incomparable types
        throw Error{
              .line = op.line
            , .message = std::format(
                  "Invalid '{}' comparison of types {} and {}"
                , op.GetRepresentation()
                , magic_enum::enum_name(leftT)
                , magic_enum::enum_name(rightT)
            )
        };
    }();

    // Invert the result if it's a !=
    return result ^ (op.type == TokenType::BangEqual);
}

static Error BinaryConversionError(std::string_view name, const Token& op, const Value& left, const Value& right) {
    return Error{
          .line = op.line
        , .message = std::format(
              "Can't perform {} for '{}' between types {} and {}"
            , name
            , op.GetRepresentation()
            , magic_enum::enum_name(left.GetType())
            , magic_enum::enum_name(right.GetType())
        )
    };
}

declare_method(Value, Evaluate, (virtual_<const Expr&>, Environment*));

static Value GetDeferredOrEval(const Expr& expr, Environment& env) {
    if (auto* value = DeferManager::Inst().GetDeferValue(expr); value != nullptr) {
        return *value;
    } else {
        return ::Evaluate(expr, &env);
    }
}

define_method(Value, Evaluate, (const BinaryExpr& expr, Environment* env)) {
    using enum TokenClass;

    Value left = env->ExtractFromLV(GetDeferredOrEval(*expr.left, *env));
    DeferGuard guardL(*expr.left, left);

    // No guard necessary for right because it's the last value calculated
    // Any bail-out (via exception) will not have a completed value here
    // So it is useless to try to defer anyways.
    Value right = env->ExtractFromLV(GetDeferredOrEval(*expr.right, *env));

    DeferGuard::MarkSuccess(guardL);

    // Special case equality to its own function
    if (expr.op.type == TokenType::EqualEqual || expr.op.type == TokenType::BangEqual) {
        return EvaluateEquality(left, right, expr.op);
    }

    switch (GetTokenClass(expr.op.type)) {
        case Arithmetic:
        case Comparison: {
            if (left.GetType() == ValueType::String && right.GetType() == ValueType::String) {
                // Support for binary expressions between strings
                return EvaluateBinaryExpr(left, right, expr.op);
            } else {
                // Support for binary expressions between numbers
                auto res = NumericConversion(left, right);

                if (not res)
                    throw BinaryConversionError("numeric conversion", expr.op, left, right);

                return EvaluateBinaryExpr(res->left, res->right, expr.op);
            }
        }
        default:
            throw std::runtime_error(std::format(
                "Invalid binary operator {}", expr.op.GetRepresentation()
            ));
    }
}

define_method(Value, Evaluate, (const UnaryExpr& expr, Environment* env)) {
    using enum TokenType;

    // Don't need to guard because it's the only value we calculate
    const Value operand = env->ExtractFromLV(GetDeferredOrEval(*expr.operand, *env));

    switch (expr.op.type) {
        case Minus: {
            if (not operand.IsArithmetic()) {
                throw Error{
                      .line = expr.op.line
                    , .message = std::format(
                          "Expected numeric operand for '-'. Got {}"
                        , magic_enum::enum_name(operand.GetType())
                    )
                };
            }

            Token multToken = expr.op;
            multToken.type = Star;

            return EvaluateBinaryExpr(-1, operand, multToken);
        } 
        case Not: {
            if (operand.GetType() != ValueType::Boolean) {
                throw Error{
                      .line = expr.op.line
                    , .message = std::format(
                          "Expected boolean operand for 'not'. Got {}"
                        , magic_enum::enum_name(operand.GetType())
                    )
                };
            }

            return not operand.GetAs<bool>();
        }
        default:
            throw std::runtime_error(std::format(
                  "Invalid unary operator {}"
                , expr.op.GetRepresentation()
            ));
    }
}

define_method(Value, Evaluate, (const GroupingExpr&, Environment*)) {
    throw std::runtime_error("GroupingExpr unused");
    // return Evaluate(*expr.expr, env);
}

define_method(Value, Evaluate, (const LiteralExpr& expr, Environment*)) {
    return expr.value;
}

define_method(Value, Evaluate, (const AssignmentExpr& expr, Environment* env)) {
    Value target = GetDeferredOrEval(*expr.target, *env);
    DeferGuard guardTarget(*expr.target, target);

    if (auto type = target.GetType(); type != ValueType::Lvalue) {
        throw Error{
              .line = expr.equal.line
            , .message = std::format(
                  "Expected lvalue for assignment target, got {} instead"
                , magic_enum::enum_name(type)
            )
        };
    }

    const Lvalue& lvalue = target.GetAs<Lvalue>();
    VarDecl* var = env->GetVar(lvalue.name);
    
    if (var == nullptr)
        throw UndefinedVariableError(lvalue.lineOfRef, lvalue.name);

    // Don't need to guard this because it's the last value we calculate
    const Value rvalue = env->ExtractFromLV(Evaluate(*expr.value, env));
    var->Set(rvalue, expr.equal.line);

    return rvalue;
}

Value AstMethods::Evaluate(const Expr& expr, Environment& env, ErrorContext&) {
    return ::Evaluate(expr, &env);
}