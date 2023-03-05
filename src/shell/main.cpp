#include <fstream>
#include <iostream>
#include "Terminal.hpp"
#include "InterpreterInterface.hpp"
#include "core/Error.hpp"
#include "core/Interpreter.hpp"
#include "core/Lexer.hpp"
#include "core/Parser.hpp"
#include "core/Statement.hpp"
#include "core/Tokens.hpp"
#include "core/AstMethods/Print.hpp"
#include "core/AstMethods/Evaluate.hpp"

using namespace dxsh;
using namespace core;
using namespace std::string_literals;

int main(int argc, char** argv) {
    yorel::yomm2::update_methods();
    
    Terminal term{};

    try {
        if (argc == 1) {
            shell::REPL(term);
        } else {
            std::ifstream file(argv[1]);

            if (not file.is_open()) {
                term.PrintError(std::format("Unable to open file '{}'", argv[1]));
                return 1;
            }

            shell::File(term, file);
        }
    } catch (const std::exception& e) {
        term.PrintError("Internal exception: "s + e.what());
        return -1;
    }
} 