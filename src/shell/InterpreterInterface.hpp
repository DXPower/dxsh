#pragma once 

#include <span>
#include "core/Statement.hpp"
#include "core/Interpreter.hpp"
#include "Terminal.hpp"


namespace dxsh {
    namespace shell {
        void InterpreterInterface(
              core::Interpreter& interpreter
            , Terminal& term
            , std::span<const std::unique_ptr<core::Statement>> statements
            , bool quitOnError
        );

        void REPL(Terminal& term);
        void File(Terminal& term, std::ifstream& file);
    }
}