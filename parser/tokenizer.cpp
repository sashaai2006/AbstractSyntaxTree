#include "tokenizer.h"
#include <regex>
#include <string>
#include <cctype>
#include "parser_exceptions.h"

const std::unordered_map<std::string_view, TokenType> Tokenizer::kTokenMap = {
    {"#", TokenType::EndOfFile},
    {"lambda", TokenType::Lambda},
    {".", TokenType::Dot},
    {"+", TokenType::BinaryOperator},
    {"-", TokenType::BinaryOperator},
    {"*", TokenType::BinaryOperator},
    {"/", TokenType::BinaryOperator},
    {"^", TokenType::BinaryOperator},
    {"(", TokenType::OpenScope},
    {")", TokenType::CloseScope},
    {"sqrt", TokenType::UnaryOperator},
    {"abs", TokenType::UnaryOperator},
    {"exp", TokenType::UnaryOperator},
    {"ln", TokenType::UnaryOperator},
    {"floor", TokenType::UnaryOperator},
    {"ceil", TokenType::UnaryOperator},
    {"round", TokenType::UnaryOperator},
    {"trunc", TokenType::UnaryOperator},
    {"sin", TokenType::UnaryOperator},
    {"cos", TokenType::UnaryOperator},
    {"tan", TokenType::UnaryOperator},
    {"ctan", TokenType::UnaryOperator},
    {"random", TokenType::UnaryOperator}
};

Tokenizer::Tokenizer(std::string_view str) : input_(str), index_(0) {}

bool Tokenizer::isEnd() const {
    return index_ >= input_.size();
}

TokenType Tokenizer::getType(std::string val) const {
    if (val == "#") {
        return TokenType::EndOfFile;
    }

    if (auto it = kTokenMap.find(val); it != kTokenMap.end()) {
        return it->second;
    }

    static const std::regex number_re(R"(^(0|[1-9][0-9]*)(\.[0-9]+)?$)");
    if (std::regex_match(val, number_re)) {
        return TokenType::Number;
    }

    static const std::regex id_re(R"(^[A-Za-z]+$)");
    if (std::regex_match(val, id_re)) {
        return TokenType::ID;
    }

    return TokenType::Error;
}

Token Tokenizer::nextToken() {
    skipWhitespace();

    if (isEnd()) {
        return {TokenType::EndOfFile, "#"};
    }

    char ch = input_[index_];

    if (isSingleCharToken(ch)) {
        return readSingleCharToken();
    }

    if (std::isalpha(static_cast<unsigned char>(ch))) {
        return readIdentifierOrKeyword();
    }

    if (std::isdigit(static_cast<unsigned char>(ch))) {
        return readNumber();
    }

    ++index_;
    return {TokenType::Error, std::string(1, ch)};
}

void Tokenizer::skipWhitespace() {
    while (!isEnd() && std::isspace(static_cast<unsigned char>(input_[index_]))) {
        ++index_;
    }
}

bool Tokenizer::isSingleCharToken(char ch) const {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/' || 
           ch == '^' || ch == '(' || ch == ')' || ch == '.';
}

Token Tokenizer::readSingleCharToken() {
    char ch = input_[index_];
    ++index_;
    std::string token_str(1, ch);
    return {kTokenMap.at(token_str), token_str};
}

Token Tokenizer::readIdentifierOrKeyword() {
    size_t start = index_;

    while (index_ < input_.size() && std::isalpha(static_cast<unsigned char>(input_[index_]))) {
        ++index_;
    }

    std::string word = std::string(input_.substr(start, index_ - start));

    if (word == "lambda") {
        return {TokenType::Lambda, word};
    }

    auto it = kTokenMap.find(word);
    if (it != kTokenMap.end()) {
        return {it->second, word};
    }

    if (word.size() == 1) {
        return {TokenType::ID, word};
    }
    return {TokenType::Error, word};
}

Token Tokenizer::readNumber() {
    size_t start = index_;
    bool has_dot = false;
    while (index_ < input_.size()) {
        char ch = input_[index_];

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            ++index_;
        } else if (ch == '.' && !has_dot) {
            has_dot = true;
            ++index_;
        } else {
            break;
        }
    }

    std::string num = std::string(input_.substr(start, index_ - start));

    TokenType type = getType(num);

    if (type != TokenType::Number) {
        return {TokenType::Error, num};
    }

    return {TokenType::Number, num};
}

void Tokenizer::reset() {
    index_ = 0;
}

std::vector<Token> Tokenizer::tokenizeAll() {
    std::vector<Token> tokens;
    reset();
    while (true) {
        Token token = nextToken();
        if (token.type == TokenType::Error) {
            throw SyntaxError("Unrecognized token: " + token.value);
        }
        tokens.push_back(token);
        if (token.type == TokenType::EndOfFile) {
            break;
        }
    }
    return tokens;
}