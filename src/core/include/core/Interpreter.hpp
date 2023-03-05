#pragma once

#include <span>
#include <sstream>
#include <stack>
#include <generator.hpp>

#include "core/Defer.hpp"
#include "core/Error.hpp"
#include "core/Environment.hpp"
#include "core/ExecutionContext.hpp"
#include "core/Statement.hpp"

namespace dxsh {
    namespace core {
        enum class RuntimeStatus {
            Run, Finish, Error
        };

        class DeferManager;

        class Interpreter {
            std::stringstream input, output;
            std::stack<ExecutionContext> callstack;
            std::unique_ptr<DeferManager> defers;
            // std::vector<Statement> statements;

            public:
            Interpreter();

            ErrorContext errors;

            void LoadProgram(std::span<const std::unique_ptr<Statement>> statements);
            std::generator<RuntimeStatus> ExecuteTop(std::span<const std::unique_ptr<Statement>> statements);
            
            void PushContext(const ExecutionContext& ctx);
            void PopContext();
            
            Environment& GetCurEnvironment();

            void GiveInput(std::string_view input);
            std::string TakeInput();

            void GiveOutput(std::string_view output);
            std::string TakeOutput();

            void ResetIO();

        };
    }
}