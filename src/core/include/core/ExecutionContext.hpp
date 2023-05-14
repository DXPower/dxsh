#pragma once

#include <span>
#include "core/Environment.hpp"
#include "core/Statement.hpp"

namespace dxsh {
    namespace core {
        enum class ExecutionStatus {
              SUCCESS
            , ERROR
            , CLOSE
            , EXIT_FUNCTION
        };

        enum class ContextType {
            Scope, Function, Script
        };

        class ExecutionContext {
            std::size_t id;
            std::span<const std::unique_ptr<Statement>> statements;
            std::size_t curPos{};

            public:
            Environment environment;
            ContextType type{};

            ExecutionContext(std::size_t id, ContextType type, decltype(statements) statements)
                : id(id), statements(statements), type(type) { }

            ExecutionStatus ExecuteOne(Interpreter& interpreter);
            int Id() const { return id; }
        };
    }
}