#pragma once
#include <stdexcept>
#include <string>

class ParserException : public std::runtime_error {
public:
    explicit ParserException(const std::string& message) 
        : std::runtime_error("Parser Error: " + message) {}
};

class SyntaxError : public ParserException {
public:
    explicit SyntaxError(const std::string& message) 
        : ParserException("Syntax error: " + message) {}
};

class UnexpectedTokenError : public ParserException {
public:
    explicit UnexpectedTokenError(const std::string& expected, const std::string& found) 
        : ParserException("Expected " + expected + ", found " + found) {}
};

class UnexpectedEOFError : public ParserException {
public:
    explicit UnexpectedEOFError() 
        : ParserException("Unexpected end of input") {}
};

class EmptyInputError : public ParserException {
public:
    explicit EmptyInputError() 
        : ParserException("Empty input") {}
};

class UnprocessedTokensError : public ParserException {
public:
    explicit UnprocessedTokensError(const std::string& token) 
        : ParserException("Unprocessed tokens starting with: " + token) {}
};

class NoRuleFoundError : public ParserException {
public:
    explicit NoRuleFoundError(const std::string& token) 
        : ParserException("No grammar rule found for token: " + token) {}
};

