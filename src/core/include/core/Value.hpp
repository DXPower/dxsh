#pragma once

#include <string>
#include <variant>

namespace dxsh {
    namespace core {
        enum class ValueType {
              Null = 0
            , Integer
            , Decimal
            , String
            , Boolean
            , Lvalue
        };

        struct Lvalue {
            int lineOfRef;
            std::string name;

            bool operator==(const Lvalue&) const = default;
            auto operator<=>(const Lvalue&) const = default;
        };

        class Value {
            std::variant<std::monostate, int, float, std::string, bool, Lvalue> value;

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