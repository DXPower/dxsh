#pragma once

#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>


namespace dxsh {
    namespace core {
        class Value;

        enum class ValueType {
              Null = 0
            , Integer
            , Decimal
            , String
            , Boolean
            , Lvalue
            , Function
        };

        struct Lvalue {
            int lineOfRef;
            std::string_view name; // Name comes from statements, non-owning is fine

            auto operator<=>(const Lvalue&) const = default;
        };

        struct Statement;
        struct Function {
            int line;
            std::string_view name; // Statements live as long as the program, non-owning is fine
            std::vector<std::string_view> params; // Same with string_view
            std::span<const std::unique_ptr<Statement>> statements; // Same with the span

            std::size_t Arity() const { return params.size(); }
        };

        class Value {
            std::variant<std::monostate, int, float, std::string, bool, Lvalue, Function> value;

            public:
            Value() = default;
            Value(auto value) : value(std::move(value)) { }

            template<typename T>
            const T& GetAs() const { return std::get<T>(value); }

            template<typename T>
            T& GetAs() { return std::get<T>(value); }

            ValueType GetType() const {
                return static_cast<ValueType>(value.index());
            }

            bool IsArithmetic() const { 
                return GetType() == ValueType::Integer || GetType() == ValueType::Decimal;
            }

            // Returns true for boolean true only
            bool IsTrue() const {
                return GetType() == ValueType::Boolean && GetAs<bool>() == true;
            }

            std::string ToString() const;
            std::string ToPrettyString() const;
        };
    }
}