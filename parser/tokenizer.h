#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

enum class TokenType {
    Number,
    ID,
    Lambda,
    Dot,
    BinaryOperator,
    UnaryOperator,
    OpenScope,
    CloseScope,
    EndOfFile,
    Error,
};

struct Token {
    TokenType type;
    std::string value;
};

class Tokenizer {
  private:
    std::string_view input_;
    size_t index_ = 0;
    static const std::unordered_map<std::string_view, TokenType> kTokenMap;

    bool isEnd() const;
    void skipWhitespace();
    bool isSingleCharToken(char ch) const;
    Token readSingleCharToken();
    Token readIdentifierOrKeyword();
    Token readNumber();
  public:
    Tokenizer(std::string_view str);
    TokenType getType(std::string val) const;
    Token nextToken();
    void reset();
    std::vector<Token> tokenizeAll();
};
