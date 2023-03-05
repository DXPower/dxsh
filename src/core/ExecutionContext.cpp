#include "core/ExecutionContext.hpp"
#include "core/Statement.hpp"
#include "core/Interpreter.hpp"

using namespace dxsh;
using namespace core;

ExecutionStatus ExecutionContext::ExecuteOne(Interpreter& interpreter) {
    // If we are already at the end of a block, close it out
    // This can occur if a block statement is the last statement of this block
    if (curPos >= statements.size())
        return ExecutionStatus::CLOSE;

    auto effect = EvaluateStatement(*statements[curPos], interpreter);

    if (not interpreter.errors.empty())
        return ExecutionStatus::ERROR;

    curPos++;

    // The statement itself handles pushing a new context,
    // however we need to exit early still to transfer
    // execution to the new context
    if (effect == StatementEffect::OpenContext)
        return ExecutionStatus::SUCCESS;

    // Triggered by break statements
    if (effect == StatementEffect::CloseContext)
        return ExecutionStatus::CLOSE;

    // If we reach the end of this context, trigger a pop of the callstack

    if (curPos < statements.size())
        return ExecutionStatus::SUCCESS;
    else
        return ExecutionStatus::CLOSE;
}