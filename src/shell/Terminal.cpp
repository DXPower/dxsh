#include <iostream>
#include <format>

#include <rang/rang.hpp>

#include "Terminal.hpp"
using namespace dxsh;
using namespace core;

void Terminal::PrintWelcome() const {
    std::cout << "Welcome to dxsh!\n\n";
}

void Terminal::PrintPrompt() const {
    std::cout << rang::fg::green << "\n" << prompt << rang::style::reset << "  ";
}

void Terminal::SetPrompt(std::string prompt) {
    this->prompt = std::move(prompt);
}

void Terminal::Print(std::string_view str) const {
    std::cout << str;
}

void Terminal::Println(std::string_view str) const {
    std::cout << str << '\n';
}


void Terminal::PrintError(std::string_view e) const {
    std::cout << rang::fg::red << e << rang::style::reset << '\n';
}

void Terminal::PrintError(const core::Error& e) const {
    PrintError(std::format("\nError! Line {:d}: {:s}", e.line, e.message));
}

void Terminal::PrintErrors(std::span<const core::Error> errors) const {
    for (const auto& e : errors) {
        PrintError(e);
    }
}

std::string Terminal::AcceptInput() const {
    std::string s;
    std::getline(std::cin, s);
    return s;
}