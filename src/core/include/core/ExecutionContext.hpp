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
        };

        class ExecutionContext {
            std::span<const std::unique_ptr<Statement>> statements;
            std::size_t curPos{};

            public:
            Environment environment;

            ExecutionContext(decltype(statements) statements)
                : statements(statements) { }

            ExecutionStatus ExecuteOne(Interpreter& interpreter);
        };
    }
}