#include <cctype>
#include <stdexcept>
#include <type_traits>

#include "core/Lexer.hpp"
#include "core/Tokens.hpp"

using namespace dxsh;
using namespace core;

static bool IsAlpha(char c) {
    return std::isalpha(c) || c == '_';
}

static bool IsAlphanumeric(char c) {
    return IsAlpha(c) || std::isdigit(c);
}

std::vector<Token> Lexer::Parse(std::string_view source) {
    this->source = source;

    curPos = 0;
    curLine = 1;

    tokens.clear();

    while (not IsAtEnd()) {
        Scan();
    }

    AddToken(TokenType::Eof);

    return std::move(tokens);
}

void Lexer::Scan() {
    char c = Advance();

    Token token {
        .line = curLine
    };

    switch (c) {
        // Single character only tokens
        case '(': AddToken(TokenType::ParenL);    break;
        case ')': AddToken(TokenType::ParenR);    break;
        case '[': AddToken(TokenType::BracketL);  break;
        case ']': AddToken(TokenType::BracketR);  break;
        case '{': AddToken(TokenType::BraceL);    break;
        case '}': AddToken(TokenType::BraceR);    break;
        case ',': AddToken(TokenType::Comma);     break;
        case ';': AddToken(TokenType::Semicolon); break;
        case '+': AddToken(TokenType::Plus);      break;
        case '-': AddToken(TokenType::Minus);     break;
        case '%': AddToken(TokenType::Percent);   break;

        // Munchable tokens
        case '>': 
            if (MatchConsume('=')) AddToken(TokenType::GreaterEqual);
            else AddToken(TokenType::Greater);
            break;
        case '<': 
            if (MatchConsume('=')) AddToken(TokenType::LessEqual);
            else AddToken(TokenType::Less);
            break;
        case '=':
            if (MatchConsume('=')) AddToken(TokenType::EqualEqual);
            else AddToken(TokenType::Equal);
            break;
        case '!':
            if (MatchConsume('=')) AddToken(TokenType::BangEqual);
            else goto ERROR_CASE;
            break;
        case '*':
            if (MatchConsume('*')) AddToken(TokenType::StarStar);
            else AddToken(TokenType::Star);
            break;
        case '.':
            if (std::isdigit(Peek())) LexNumber(LeadingDecimal::Yes);
            else AddToken(TokenType::Dot);
            break;
        case '/': 
            if (MatchConsume('/')) SkipToNewline(); // Throw away all comments
            else AddToken(TokenType::Slash);
            break;
        
        // Literals
        case '"': LexString(); break;

        // Whitespace
        case '\n':
            curLine++;
        case '\t':
        case '\r':
        case ' ':
            break; // Ignore all whitespace

        default: 
            if (IsAlpha(c)) {
                LexIdentifier();
                break;
            } else if (std::isdigit(c)) {
                LexNumber(LeadingDecimal::No);
                break;
            }

            // Error out
            ERROR_CASE:
            errors->push_back({curLine, std::format("Unknown token {}", c)});
    }
}

void Lexer::LexString() {
    std::size_t start = curPos - 1;
    int startingLine = curLine;

    while (Peek() != '"') {
        if (IsAtEnd()) [[unlikely]] {
            errors->push_back(Error{
                  .line = curLine
                , .message = std::format("Unterminated string literal (starting at line {})", startingLine)
            });

            return;
        }

        Advance();
    }

    Advance(); // The terminating "

    std::string_view lexeme = std::string_view(source.begin() + start, source.begin() + curPos);
    
    if (lexeme.size() > 2) [[likely]] {
        AddToken(
              TokenType::String
            , std::string(lexeme)
            , std::string(lexeme.substr(1, lexeme.size() - 2))
        );
    } else {
        AddToken(
              TokenType::String
            , std::string(lexeme)
            , ""
        );
    }
}

void Lexer::LexIdentifier() {
    std::size_t start = curPos - 1;

    while (IsAlphanumeric(Peek())) {
        Advance();
    }

    std::string_view lexeme = source.substr(start, curPos - start);
    auto type = GetPotentialKeywordTokenType(lexeme);

    AddToken(type, std::string(lexeme), std::monostate{});
}

void Lexer::LexNumber(LeadingDecimal hasLeadingDecimal) {
    std::size_t start = curPos - 1;
    
    if (hasLeadingDecimal == LeadingDecimal::No) {
        // Grab all following digits
        while (std::isdigit(Peek())) {
            Advance();
        }

        // If we found a non-digit, non-decimal point, then we have an integer
        if (Peek() != '.') {
            ParseAndAddNumber<int>(source.substr(start, curPos - start));
            return;
        }

        Advance();
    }

    // Consume all fractional parts and decimal points
    while (std::isdigit(Peek()) || Peek() == '.') {
        Advance();
    }

    std::string_view literal = source.substr(start, curPos - start);

    // Decimals can't end in a decimal point (like 123.),
    if (literal.ends_with('.')) {
        errors->push_back({
              .line = curLine
            , .message = std::format(
                  "Invalid decimal literal '{:s}' (can't end in decimal point)" 
                , literal
            )
        });

        return;
    }

    // We believe we have a valid decimal literal, try to parse and add it
    ParseAndAddNumber<float>(literal);
}

template<typename T>
void Lexer::ParseAndAddNumber(std::string_view str) {
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>);

    constexpr std::string_view literalName = []() {
        if constexpr (std::is_same_v<T, int>) return "integer";
        else if constexpr (std::is_same_v<T, float>) return "decimal";
    }();

    T value;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

    if (ec == std::errc::invalid_argument) [[unlikely]] {
        errors->push_back({
              .line = curLine
            , .message = std::format("Invalid {:s} literal '{:s}'", literalName, str)
        });
    } else if (ec == std::errc::result_out_of_range) [[unlikely]] {
        errors->push_back({
              .line = curLine
            , .message = std::format("{:s} literal '{:s}' out of range", literalName, str)

        });
    } else if (ptr != str.data() + str.size()) [[unlikely]] {
        errors->push_back({
              .line = curLine
            , .message = std::format(
                  "Invalid {:s} literal '{:s}' (from '{:s}')"
                , literalName
                , str
                , std::string_view(ptr, str.data() + str.size())
            )
        });
    }

    AddToken(
          literalName == "integer" ? TokenType::Integer : TokenType::Decimal
        , std::string(str)
        , value
    );
}

template void Lexer::ParseAndAddNumber<int>(std::string_view str);
template void Lexer::ParseAndAddNumber<float>(std::string_view str);

void Lexer::AddToken(TokenType type) {
    tokens.push_back(Token{
          .type = type
        , .line = curLine
    });
}

void Lexer::AddToken(TokenType type, std::string lexeme, decltype(Token::literal) literal) {
    tokens.push_back(Token{
          .type = type
        , .line = curLine
        , .lexeme = std::move(lexeme)
        , .literal = std::move(literal)
    });
}

void Lexer::SkipToNewline() {
    while (not IsAtEnd() and Peek() != '\n') {
        Advance();
    };
}

char Lexer::Advance() {
    return source.at(curPos++);
}

char Lexer::Peek() const {
    if (IsAtEnd()) return '\0';
    return source.at(curPos);
}

bool Lexer::MatchConsume(char c) {
    if (IsAtEnd()) return false;
    if (Peek() != c) return false;

    curPos++;
    return true;
}

bool Lexer::IsAtEnd() const {
    return curPos >= source.size();
}
