#pragma once

#include <unordered_map>
#include "Error.hpp"
#include "Value.hpp"

namespace dxsh {
    namespace core {
        class Environment;

        struct VarDecl {
            std::string name;

            private:
            Value value{};
            int lineOfDecl{};
            int lineOfLastAssign{};

            public:
            const Value& GetValue() const { return value; };
            int GetLineOfDecl() const { return lineOfDecl; };
            int GetLineOfLastAssign() const { return lineOfLastAssign; }

            void Set(const Value& value, int line);

            friend Environment;
        };

        class Environment {
            Environment* parent = nullptr;
            std::unordered_map<std::string, VarDecl> variables;

            public:
            Environment() = default;

            Environment MakeChild();

            // Returns nullptr if var doesn't exist
            // VarDecl* GetVar(std::string_view name);
            VarDecl* GetVar(std::string_view name);
            
            // Returns nullptr if var doesn't exist
            // VarDecl* GetVar(std::string_view name);
            const VarDecl* GetVar(std::string_view name) const;

            // Will assign if var already exists
            void CreateOrAssignVar(std::string_view name, const Value& value, int line);

            // If v is an lvalue, will return its true value retrieved from this environment
            // Else, returns v
            // If variable is not found in this context, throws error without line info
            const Value& ExtractFromLV(const Value& v) const;
        };

        struct UndefinedVariableError : Error {
            UndefinedVariableError(int line, std::string_view name);
        };
    }
}