#pragma once

#include <string_view>

#include "Tokens.hpp"
#include "Error.hpp"

namespace dxsh {
    namespace core {
        class Lexer {
            enum class LeadingDecimal { No, Yes };

            ErrorContext* errors;

            std::vector<Token> tokens;

            std::string_view source;
            std::size_t curPos{};
            int curLine{};

            public:
            Lexer(ErrorContext& errors) : errors(&errors) { };

            std::vector<Token> Parse(std::string_view source);

            private:
            void Scan();

            // For non-value tokens
            void AddToken(TokenType type);
            // For literal tokens
            void AddToken(TokenType type, std::string lexeme, decltype(Token::literal) literal);

            char Advance();
            char Peek() const;
            bool MatchConsume(char c);

            void LexString();
            void LexIdentifier();
            void LexNumber(LeadingDecimal hasLeadingDecimal);

            template<typename T>
            void ParseAndAddNumber(std::string_view str);

            void SkipToNewline();

            bool IsAtEnd() const;
        };
    }
}