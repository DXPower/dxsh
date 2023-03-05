#include "core/Interpreter.hpp"
#include "core/Defer.hpp"
#include "core/ExecutionContext.hpp"
#include "core/Statement.hpp"

using namespace std::string_literals;
using namespace dxsh;
using namespace core;

Interpreter::Interpreter() {
    defers = std::make_unique<DeferManager>();
    DeferManager::Inst(*defers);
}

void Interpreter::LoadProgram(std::span<const std::unique_ptr<Statement>> statements) {
    // Setup the global execution context
    callstack = {};
    callstack.emplace(statements);
}

std::generator<RuntimeStatus> Interpreter::ExecuteTop(std::span<const std::unique_ptr<Statement>> statements) {
    while (not callstack.empty()) {
        auto status = callstack.top().ExecuteOne(*this);

        if (status == ExecutionStatus::ERROR) {
            co_yield RuntimeStatus::Error;
            co_return;
        }

        // If an execution context has finished normally, pop it off
        if (status == ExecutionStatus::CLOSE) {
            PopContext();
        }

        co_yield RuntimeStatus::Run;
    }

    co_yield RuntimeStatus::Finish;
}

Environment& Interpreter::GetCurEnvironment() {
    return callstack.top().environment;
}

void Interpreter::PushContext(const ExecutionContext& ctx) {
    Environment& curEnv = GetCurEnvironment();

    callstack.push(ctx);
    callstack.top().environment = curEnv.MakeChild();
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