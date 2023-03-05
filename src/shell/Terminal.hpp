#pragma once

#include <span>
#include <string>
#include <string_view>
#include "core/Error.hpp"

namespace dxsh {
    class Terminal {
        std::string prompt = "dxsh$";

        public:
        void PrintWelcome() const;

        void PrintPrompt() const;
        void SetPrompt(std::string prompt);

        void Print(std::string_view str) const;
        void Println(std::string_view str) const;

        void PrintError(std::string_view e) const;
        void PrintError(const core::Error& e) const;
        void PrintErrors(std::span<const core::Error> errors) const;

        std::string AcceptInput() const;
    };
}