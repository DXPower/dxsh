#include <format>
#include "core/Environment.hpp"
#include "core/Value.hpp"

using namespace dxsh;
using namespace core;

void VarDecl::Set(const Value& value, int line) {
    this->value = value;
    lineOfLastAssign = line;
}

const VarDecl* Environment::GetVar(std::string_view name) const {
    return const_cast<Environment*>(this)->GetVar(name);
}

VarDecl* Environment::GetVar(std::string_view name) {
    auto it = variables.find(std::string(name));

    if (it == variables.end()) {
        if (parent != nullptr) {
            return parent->GetVar(name);
        } else {
            return nullptr;
        }
    }

    return &it->second;
}

Environment Environment::MakeChild() {
    Environment env;
    env.parent = this;
    return env;
}


void Environment::CreateOrAssignVar(std::string_view name, const Value& value, int line) {
    auto it = variables.find(std::string(name));

    if (it != variables.end()) {
        it->second.value = value;
        it->second.lineOfLastAssign = line;
    } else {
        VarDecl var{};
        var.name = std::string(name);
        var.value = value;
        var.lineOfDecl = line;

        variables.emplace(var.name, var);
    }
}

const Value& Environment::ExtractFromLV(const Value& v) const {
    if (v.GetType() != ValueType::Lvalue)
        return v;

    Lvalue lv = v.GetAs<Lvalue>();
    const VarDecl* var = GetVar(lv.name);

    if (var == nullptr)
        throw UndefinedVariableError(lv.lineOfRef, v.GetAs<Lvalue>().name);

    return var->GetValue();
}

UndefinedVariableError::UndefinedVariableError(int line, std::string_view name)
    : Error{
          .line = line
        , .message = std::format("Use of undefined variable '{}'", name)
    }
{ }