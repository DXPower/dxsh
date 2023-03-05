#include <iostream>
#include <magic_enum/magic_enum_container.hpp>

#include "core/Tokens.hpp"

using namespace dxsh;
using namespace core;

static std::unordered_map<std::string_view, TokenType> keywordTokenMap = {
      { "and",      TokenType::And }
    , { "or",       TokenType::Or }
    , { "not",      TokenType::Not }
    , { "print",    TokenType::Print }
    , { "true",     TokenType::True }
    , { "false",    TokenType::False }
    , { "for",      TokenType::For }
    , { "if",       TokenType::If }
    , { "else",     TokenType::Else }
    , { "while",    TokenType::While }
    , { "func",     TokenType::Function }
    , { "return",   TokenType::Return }
    , { "var",      TokenType::Var }
    , { "null",     TokenType::Null }
};

static constexpr auto tokenTypeReprs = []() {
    using enum TokenType;

    magic_enum::containers::array<TokenType, std::string_view> reprs{};

    reprs[ParenL]       = "(";
    reprs[ParenR]       = ")";
    reprs[BraceL]       = "{";
    reprs[BraceR]       = "}";
    reprs[BracketL]     = "[";
    reprs[BracketR]     = "]";
    reprs[Comma]        = ",";
    reprs[Dot]          = ".";
    reprs[Semicolon]    = ";";
    reprs[Plus]         = "+";
    reprs[Minus]        = "-";
    reprs[Star]         = "*";
    reprs[Slash]        = "/";
    reprs[Percent]      = "%";
    reprs[StarStar]     = "**";
    reprs[EqualEqual]   = "==";
    reprs[BangEqual]    = "!=";
    reprs[Greater]      = ">";
    reprs[GreaterEqual] = ">=";
    reprs[Less]         = "<";
    reprs[LessEqual]    = "<=";
    reprs[Equal]        = "=";
    reprs[And]          = "and";
    reprs[Or]           = "or";
    reprs[Not]          = "not";
    reprs[True]         = "true";
    reprs[False]        = "false";
    reprs[Function]     = "func";
    reprs[For]          = "for";
    reprs[If]           = "if";
    reprs[Else]           = "else";
    reprs[While]        = "while";
    reprs[Null]         = "null";
    reprs[Return]       = "return";
    reprs[Var]          = "var";
    reprs[Print]        = "print";
    reprs[Eof]          = "[EOF]";

    return reprs;
}();

static constexpr auto tokenClasses = []() {
    using enum TokenType;
    using enum TokenClass;

    magic_enum::containers::array<TokenType, TokenClass> reprs{};

    reprs[ParenL]       = Bracket;
    reprs[ParenR]       = Bracket;
    reprs[BraceL]       = Bracket;
    reprs[BraceR]       = Bracket;
    reprs[BracketL]     = Bracket;
    reprs[BracketR]     = Bracket;
    reprs[Comma]        = Separator;
    reprs[Dot]          = Separator;
    reprs[Semicolon]    = Separator;
    reprs[Plus]         = Arithmetic;
    reprs[Minus]        = Arithmetic;
    reprs[Star]         = Arithmetic;
    reprs[Slash]        = Arithmetic;
    reprs[Percent]      = Arithmetic;
    reprs[StarStar]     = Arithmetic;
    reprs[EqualEqual]   = Comparison;
    reprs[BangEqual]    = Comparison;
    reprs[Greater]      = Comparison;
    reprs[GreaterEqual] = Comparison;
    reprs[Less]         = Comparison;
    reprs[LessEqual]    = Comparison;
    reprs[And]          = Logic;
    reprs[Or]           = Logic;
    reprs[Not]          = Logic;
    reprs[String]       = Literal;
    reprs[Integer]      = Literal;
    reprs[Decimal]      = Literal;
    reprs[True]         = Literal;
    reprs[False]        = Literal;
    reprs[Null]         = Literal;
    reprs[Function]     = Keyword;
    reprs[For]          = Keyword;
    reprs[If]           = Keyword;
    reprs[Else]         = Keyword;
    reprs[While]        = Keyword;
    reprs[Return]       = Keyword;
    reprs[Var]          = Keyword;
    reprs[Print]        = SpecialFunction;
    reprs[Identifier]   = Misc;
    reprs[Equal]        = Misc;
    reprs[Eof]          = Misc;

    return reprs;
}();

TokenType core::GetPotentialKeywordTokenType(std::string_view lexeme) {
    auto it = keywordTokenMap.find(lexeme);

    if (it != keywordTokenMap.end())
        return it->second;
    else
        return TokenType::Identifier;
}

TokenClass core::GetTokenClass(TokenType type) {
    return tokenClasses[type];
}

std::string_view Token::GetRepresentation() const {
    if (not lexeme.empty()) {
        return lexeme;
    } else {
        return tokenTypeReprs[type];
    }
}

std::ostream& core::operator<<(std::ostream& out, const Token& t) {
    out << std::format("{}", t);
    return out;
}