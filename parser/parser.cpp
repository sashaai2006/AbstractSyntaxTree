#include "parser.h"
#include <algorithm>
#include <utility>

Parser::Parser(const std::string& input) {
    Tokenizer tokenizer(input);
    tokens_ = tokenizer.tokenizeAll();
    if (tokens_.empty()) {
        throw EmptyInputError();
    }
    if (tokens_.back().type != TokenType::EndOfFile) {
        tokens_.push_back(Token{TokenType::EndOfFile, "#"});
    }
}

AST Parser::buildAST() {
    current_index_ = 0;
    AST::NodePtr root = parseExpression();

    if (!isAtEnd()) {
        throw UnprocessedTokensError(peek().value);
    }

    AST ast;
    ast.setRoot(root);
    return ast;
}

const Token& Parser::peek() const {
    if (current_index_ < tokens_.size()) {
        return tokens_[current_index_];
    }
    static Token eofToken{TokenType::EndOfFile, "#"};
    return eofToken;
}

const Token& Parser::previous() const {
    if (current_index_ == 0) {
        static Token dummy{TokenType::EndOfFile, "#"};
        return dummy;
    }
    return tokens_[current_index_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++current_index_;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return false;
    }
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::matchBinaryOperator(const std::vector<std::string>& operators) {
    if (!check(TokenType::BinaryOperator)) {
        return false;
    }
    const std::string& value = peek().value;
    if (std::find(operators.begin(), operators.end(), value) == operators.end()) {
        return false;
    }
    advance();
    return true;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    throw UnexpectedTokenError(message, peek().value);
}

AST::NodePtr Parser::parseExpression() {
    return parseAddition();
}

AST::NodePtr Parser::parseAddition() {
    AST::NodePtr node = parseMultiplication();
    while (matchBinaryOperator({"+", "-"})) {
        Token op = previous();
        AST::NodePtr right = parseMultiplication();
        node = makeBinaryNode(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

AST::NodePtr Parser::parseMultiplication() {
    AST::NodePtr node = parseExponentiation();
    while (matchBinaryOperator({"*", "/"})) {
        Token op = previous();
        AST::NodePtr right = parseExponentiation();
        node = makeBinaryNode(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

AST::NodePtr Parser::parseExponentiation() {
    AST::NodePtr node = parseUnary();
    if (matchBinaryOperator({"^"})) {
        Token op = previous();
        AST::NodePtr right = parseExponentiation();
        node = makeBinaryNode(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

AST::NodePtr Parser::parseUnary() {
    if (check(TokenType::UnaryOperator)) {
        Token op = advance();
        consume(TokenType::OpenScope, "'(' after unary operator");
        AST::NodePtr operand = parseExpression();
        consume(TokenType::CloseScope, "')' after unary operator");
        return makeUnaryNode(std::move(op), std::move(operand));
    }
    if (check(TokenType::BinaryOperator) && (peek().value == "-" || peek().value == "+")) {
        Token op = advance();
        AST::NodePtr operand = parseUnary();
        AST::NodePtr zero_node = AST::createLeaf(Token{TokenType::Number, "0"});
        return makeBinaryNode(std::move(op), std::move(zero_node), std::move(operand));
    }
    if (match(TokenType::Lambda)) {
        Token lambda_token = previous();
        Token identifier = consume(TokenType::ID, "identifier after lambda");
        consume(TokenType::Dot, "'.' after lambda parameter");
        AST::NodePtr body = parseExpression();
        AST::NodePtr parameter_node = AST::createLeaf(std::move(identifier));
        AST::NodePtr lambda_node = AST::createNode(std::move(lambda_token), std::move(parameter_node), std::move(body));
        return lambda_node;
    }
    return parsePrimary();
}

AST::NodePtr Parser::parsePrimary() {
    if (match(TokenType::Number) || match(TokenType::ID)) {
        return AST::createLeaf(previous());
    }

    if (match(TokenType::OpenScope)) {
        AST::NodePtr expr = parseExpression();
        consume(TokenType::CloseScope, "closing parenthesis");
        return expr;
    }

    throw SyntaxError("Unexpected token: " + peek().value);
}

AST::NodePtr Parser::makeBinaryNode(Token op, AST::NodePtr left, AST::NodePtr right) {
    if (!left || !right) {
        throw ParserException("Binary operator missing operand: " + op.value);
    }
    return AST::createNode(std::move(op), std::move(left), std::move(right));
}

AST::NodePtr Parser::makeUnaryNode(Token op, AST::NodePtr child) {
    if (!child) {
        throw ParserException("Unary operator missing operand: " + op.value);
    }
    return AST::createNode(std::move(op), std::move(child), nullptr);
}