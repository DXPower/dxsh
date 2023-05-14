#pragma once

#include <functional>
#include <span>
#include <sstream>
#include <stack>
#include <generator.hpp>

#include "core/Error.hpp"
#include "core/Environment.hpp"
#include "core/ExecutionContext.hpp"
#include "core/Statement.hpp"

namespace dxsh {
    namespace core {
        enum class RuntimeStatus {
            RanStatement, ClosedContext, Error
        };

        class Interpreter {
            std::stringstream input, output;
            std::stack<ExecutionContext> callstack;
            std::stack<Value> returnValues;
            std::function<void(void)> interpreterInterface;
            bool isExitingFunction{};

            public:
            ErrorContext errors;

            void LoadProgram(std::span<const std::unique_ptr<Statement>> statements);
            void LoadInterface(std::function<void(void)> interface);

            void RunInterface();

            std::generator<RuntimeStatus> ExecuteTopContext();
            
            ExecutionContext& PushContext(ContextType type, std::span<const std::unique_ptr<Statement>> statements);
            void PopContext();

            // Push a return value into the interpreter's stack, defaults to null
            void PushReturn(Value v = {});
            Value PopReturn();
            bool IsExitingFunction() const { return isExitingFunction; };

            Environment& GetCurEnvironment();
            

            void GiveInput(std::string_view input);
            std::string TakeInput();

            void GiveOutput(std::string_view output);
            std::string TakeOutput();

            void ResetIO();

        };
    }
}