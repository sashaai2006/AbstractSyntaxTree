#pragma once

#include "../ast/ast.h"
#include "tokenizer.h"
#include "parser_exceptions.h"
#include <string>
#include <vector>

class Parser {
  public:
    explicit Parser(const std::string& input);
    AST buildAST();

  private:
    std::vector<Token> tokens_;
    size_t current_index_ = 0;

    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchBinaryOperator(const std::vector<std::string>& operators);
    Token consume(TokenType type, const std::string& message);

    AST::NodePtr parseExpression();
    AST::NodePtr parseAddition();
    AST::NodePtr parseMultiplication();
    AST::NodePtr parseExponentiation();
    AST::NodePtr parseUnary();
    AST::NodePtr parsePrimary();

    static AST::NodePtr makeBinaryNode(Token op, AST::NodePtr left, AST::NodePtr right);
    static AST::NodePtr makeUnaryNode(Token op, AST::NodePtr child);
};