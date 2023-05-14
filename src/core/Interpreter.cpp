#include "core/Interpreter.hpp"
#include "core/ExecutionContext.hpp"
#include "core/Statement.hpp"

using namespace std::string_literals;
using namespace dxsh;
using namespace core;

void Interpreter::LoadProgram(std::span<const std::unique_ptr<Statement>> statements) {
    // Setup the global execution context
    callstack = {};
    PushContext(ContextType::Script, statements);
}

void Interpreter::LoadInterface(std::function<void ()> interface) {
    interpreterInterface = std::move(interface);
}

void Interpreter::RunInterface() {
    interpreterInterface();
}

std::generator<RuntimeStatus> Interpreter::ExecuteTopContext() {
    using enum ExecutionStatus;

    while (true) {
        if (!isExitingFunction) {
            auto status = callstack.top().ExecuteOne(*this);

            switch (status) {
                case SUCCESS:
                    co_yield RuntimeStatus::RanStatement;
                    break;
                case CLOSE:
                    PopContext();
                    co_yield RuntimeStatus::ClosedContext;
                    co_return;
                case EXIT_FUNCTION: {
                    // Return statement was run, we will reenter this coroutine
                    // and begin the exit function process
                    co_yield RuntimeStatus::RanStatement;
                    isExitingFunction = true;
                    break;
                }
                case ERROR:
                    co_yield RuntimeStatus::Error;
                    co_return;
            }
        } else {
            if (callstack.top().type == ContextType::Script) {
                errors.push_back(Error{ 
                      .line = 0
                    , .message = "Returning from top-level not implemented"});

                co_yield RuntimeStatus::Error;
                co_return;
            }

            // Continue to exit the current function until we reach the callstack of said function
            isExitingFunction = callstack.top().type != ContextType::Function;

            PopContext();
            co_yield RuntimeStatus::ClosedContext;
            co_return;
        }
    }
}

Environment& Interpreter::GetCurEnvironment() {
    return callstack.top().environment;
}

ExecutionContext& Interpreter::PushContext(ContextType type, std::span<const std::unique_ptr<Statement>> statements) {
    ExecutionContext newFrame(callstack.size(), type, statements);

    if (callstack.size() != 0) {
        Environment& curEnv = GetCurEnvironment();
        newFrame.environment = curEnv.MakeChild();
    }

    callstack.push(newFrame);
    return callstack.top();
}

void Interpreter::PopContext() {
    callstack.pop();
}

void Interpreter::PushReturn(Value value) {
    returnValues.push(value);
}

Value Interpreter::PopReturn() {
    if (returnValues.size() == 0)
        throw std::runtime_error("Attempt to get return value when none exists");

    Value v = returnValues.top();
    returnValues.pop();

    return v;
}

void Interpreter::GiveInput(std::string_view input) {
    this->input << input;
}

std::string Interpreter::TakeInput() {
    std::string ret = input.str();
    input = {};
    return ret;
}

void Interpreter::GiveOutput(std::string_view output) {
    this->output << output;
}

std::string Interpreter::TakeOutput() {
    std::string ret = output.str();
    output = {};
    return ret;
}

void Interpreter::ResetIO() {
    input = {};
    output = {};
}