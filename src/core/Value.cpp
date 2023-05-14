#include <format>
#include <magic_enum/magic_enum_container.hpp>
#include "core/Value.hpp"

using namespace dxsh;
using namespace core;

std::string Value::ToString() const {
    using enum ValueType;

    switch (GetType()) {
        case Null:       return "null";
        case Integer:    return std::to_string(std::get<int>(value));
        case Decimal:    return std::to_string(std::get<float>(value));
        case String:     return std::get<std::string>(value);
        case Boolean:    return std::get<bool>(value) ? "true" : "false";
        case Lvalue:     return std::string(std::get<core::Lvalue>(value).name);
        case Function:   return std::format("[Function: {}]", std::get<core::Function>(value).name);
    }
}

std::string Value::ToPrettyString() const {
    using enum ValueType;

    std::string_view name;

    switch (GetType()) {
        case Integer:    name = "Integer"; break;
        case Decimal:    name = "Decimal"; break;
        case String:     name = "String"; break;
        case Boolean:    name = "Boolean"; break;
        case Lvalue:     name = "Lvalue"; break;
        case Function:   return ToString();
        case Null:       return "(null)";
    }

    return std::format("{}: {}", name, ToString());
}