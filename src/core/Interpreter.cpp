#include "core/Interpreter.hpp"
#include "core/ExecutionContext.hpp"
#include "core/Statement.hpp"

using namespace std::string_literals;
using namespace dxsh;
using namespace core;

void Interpreter::LoadProgram(std::span<const std::unique_ptr<Statement>> statements) {
    // Setup the global execution context
    callstack = {};
    PushContext(statements);
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
        auto status = callstack.top().ExecuteOne(*this);

        switch (status) {
            case SUCCESS:
                co_yield RuntimeStatus::RanStatement;
                break;
            case CLOSE:
                PopContext();
                co_yield RuntimeStatus::ClosedContext;
                co_return;
            case ERROR:
                co_yield RuntimeStatus::Error;
                co_return;
        }
    }
}

Environment& Interpreter::GetCurEnvironment() {
    return callstack.top().environment;
}

ExecutionContext& Interpreter::PushContext(std::span<const std::unique_ptr<Statement>> statements) {
    ExecutionContext newFrame{statements};

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