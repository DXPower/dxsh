#include "core/ExecutionContext.hpp"
#include "core/Statement.hpp"
#include "core/Interpreter.hpp"

using namespace dxsh;
using namespace core;

ExecutionStatus ExecutionContext::ExecuteOne(Interpreter& interpreter) {
    // Close out this execution block once we reach the end
    if (curPos >= statements.size())
        return ExecutionStatus::CLOSE;

    auto effect = EvaluateStatement(*statements[curPos], interpreter);

    if (not interpreter.errors.empty())
        return ExecutionStatus::ERROR;

    curPos++;

    // Triggered by break
    if (effect == StatementEffect::CloseContext)
        return ExecutionStatus::CLOSE;
    // Triggered by return
    else if (effect == StatementEffect::ExitFunction)
        return ExecutionStatus::EXIT_FUNCTION;

    // If we reach the end of this context, trigger a pop of the callstack
    return ExecutionStatus::SUCCESS;
}