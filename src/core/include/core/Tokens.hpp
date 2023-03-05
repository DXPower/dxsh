#pragma once

#include <variant>
#include <string>
#include <format>
#include <magic_enum/magic_enum.hpp>
#include <iosfwd>
#include <unordered_map>
#include <optional>

namespace dxsh {
    namespace core {
        enum class TokenType {
            // Brackets
              ParenL, ParenR, BraceL, BraceR, BracketL, BracketR
            // Separators
            , Comma, Dot, Semicolon
            //  Arithmetic
            , Plus, Minus, Star, Slash, Percent, StarStar
            // Comparison
            , EqualEqual, BangEqual, Greater, GreaterEqual, Less, LessEqual
            // Logic
            , And, Or, Not
            // Literals
            , String, Integer, Decimal, True, False, Null
            // Keywords
            , Function, For, If, Else, While, Return, Var
            // Special functions
            , Print
            // Misc
            , Identifier, Equal, Eof
        };

        enum class TokenClass {
            Bracket, Separator, Arithmetic, Comparison, Logic, Literal, Keyword, SpecialFunction, Misc
        };

        // Returns Identifier if it is not a keyword
        TokenType GetPotentialKeywordTokenType(std::string_view lexeme);
        TokenClass GetTokenClass(TokenType type);

        struct Token {
            TokenType type{};
            int line{};
            std::string lexeme{};
            std::variant<std::monostate, std::string, int, float> literal{};

            std::string_view GetRepresentation() const;

            friend std::ostream& operator<<(std::ostream&, const Token& t);
        };

        
    }
}

namespace std {
    template<typename CharT>
    struct formatter<dxsh::core::Token, CharT> : std::formatter<std::string, CharT> {
        template<class FormatContext>
        auto format(const dxsh::core::Token& t, FormatContext& fc) const {
            using formatter = std::formatter<std::string, CharT>;

            std::string tokenVal;

            if (not std::holds_alternative<std::monostate>(t.literal)) {
                return formatter::format(
                    std::format(
                            "[Type: {:s}; Line: {:d}; Lexeme: {:s}]"
                        , magic_enum::enum_name(t.type)
                        , t.line
                        , t.lexeme
                    ), fc
                );
            } else {
                return formatter::format(
                    std::format(
                            "[Type: {:s}; Line: {:d}]"
                        , magic_enum::enum_name(t.type)
                        , t.line
                    ), fc
                );
            }
        }
    };
    
}