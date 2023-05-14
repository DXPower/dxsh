#include <fstream>
#include "core/Lexer.hpp"
#include "core/Interpreter.hpp"
#include "core/Parser.hpp"
#include "InterpreterInterface.hpp"

using namespace dxsh;
using namespace shell;
using namespace core;

void shell::InterpreterInterface(
      Interpreter& interpreter
    , Terminal& term
    , std::span<const std::unique_ptr<Statement>> statements
    , bool quitOnError) {

    interpreter.LoadProgram(statements);

    interpreter.LoadInterface([&interpreter, &term, quitOnError]() {
        for (auto res : interpreter.ExecuteTopContext()) {
            switch (res) {
                case RuntimeStatus::RanStatement:
                    // term.Println("Ran!");
                    term.Print(interpreter.TakeOutput());
                    break;
                case RuntimeStatus::ClosedContext:
                    return;
                case RuntimeStatus::Error:
                    term.PrintErrors(interpreter.errors);
                    interpreter.ResetIO();

                    if (quitOnError)
                        throw std::runtime_error("Interpreter quitting...");

                    break;
            }
        }
    });

    interpreter.RunInterface();
}

void shell::REPL(Terminal& term) {
    Interpreter interpreter;
    auto& errors = interpreter.errors;

    term.PrintWelcome();

    while (true) {
        errors.clear();

        term.PrintPrompt();

        std::string input = term.AcceptInput();

        if (not input.ends_with(';'))
            input.push_back(';');

        core::Lexer lexer(errors);
        const auto tokens = lexer.Parse(input);

        if (not errors.empty()) {
            term.PrintErrors(errors);
            continue;
        }

        core::Parser parser(errors);
        const auto statements = parser.Parse(tokens);

        if (not errors.empty()) {
            term.PrintErrors(errors);
            continue;
        }

        shell::InterpreterInterface(interpreter, term, statements, false);
    }
}

static std::string FileToString(std::ifstream& file) {
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void shell::File(Terminal& term, std::ifstream& file) {
    const std::string contents = FileToString(file);

    Interpreter interpreter;
    auto& errors = interpreter.errors;

    Lexer lexer(errors);
    const auto tokens = lexer.Parse(contents);

    if (not errors.empty()) {
        term.PrintErrors(errors);
        return;
    }
    
    Parser parser(errors);
    const auto statements = parser.Parse(tokens);

    if (not errors.empty()) {
        term.PrintErrors(errors);
        return;
    }
    
    shell::InterpreterInterface(interpreter, term, statements, true);
}