#include "pda.h"
#include "parser_exceptions.h"

PDA::PDA(const std::vector<Token>& input_tokens) : tokens_(input_tokens) {
    InitGrammar();
}

void PDA::InitGrammar() {
    grammar_[NonTerminal::E] = {
        {NonTerminal::T, NonTerminal::E_}
    };

    grammar_[NonTerminal::E_] = {
        {TokenType::BinaryOperator, NonTerminal::T, NonTerminal::E_},
        {}
    };

    grammar_[NonTerminal::T] = {
        {NonTerminal::F, NonTerminal::T_}
    };

    grammar_[NonTerminal::T_] = {
        {TokenType::BinaryOperator, NonTerminal::F, NonTerminal::T_},
        {}
    };

    grammar_[NonTerminal::F] = {
        {TokenType::Number},
        {TokenType::ID},
        {TokenType::OpenScope, NonTerminal::E, TokenType::CloseScope},
        {TokenType::UnaryOperator, NonTerminal::F},
        {TokenType::Lambda, TokenType::Dot, TokenType::ID}
    };
}

bool PDA::IsTerminal(const StackSymbol& symbol) const {
    return std::holds_alternative<TokenType>(symbol);
}

bool PDA::IsNonTerminal(const StackSymbol& symbol) const {
    return std::holds_alternative<NonTerminal>(symbol);
}

std::vector<StackSymbol> PDA::SelectRule(NonTerminal non_terminal, TokenType lookahead) {
    auto& rules = grammar_[non_terminal];

    switch (non_terminal) {
        case NonTerminal::E:
            return rules[0];

        case NonTerminal::E_:
            if (lookahead == TokenType::BinaryOperator) {
                if (current_index_ < tokens_.size()) {
                    const std::string& op = tokens_[current_index_].value;
                    if (op == "+" || op == "-") {
                        return rules[0];
                    }
                }
            }
            return rules[1];

        case NonTerminal::T:
            return rules[0];

        case NonTerminal::T_:
            if (lookahead == TokenType::BinaryOperator) {
                if (current_index_ < tokens_.size()) {
                    const std::string& op = tokens_[current_index_].value;
                    if (op == "*" || op == "/" || op == "^") {
                        return rules[0];
                    }
                }
            }
            return rules[1];

        case NonTerminal::F:
            if (lookahead == TokenType::Number) {
                return rules[0];
            } else if (lookahead == TokenType::ID) {
                return rules[1];
            } else if (lookahead == TokenType::OpenScope) {
                return rules[2];
            } else if (lookahead == TokenType::UnaryOperator) {
                if (current_index_ < tokens_.size()) {
                    const std::string& op = tokens_[current_index_].value;
                    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "^") {
                        throw SyntaxError("Unexpected binary operator '" + op + "' in operand position");
                    }
                }
                return rules[3];
            } else if (lookahead == TokenType::Lambda) {
                return rules[4];
            } else if (lookahead == TokenType::BinaryOperator) {
                if (current_index_ < tokens_.size()) {
                    throw SyntaxError("Unexpected operator '" + tokens_[current_index_].value +
                                      "' where operand expected (number, variable or expression)");
                }
                return {};
            }
            break;
    }

    return {};
}

bool PDA::Match(TokenType expected) {
    if (current_index_ >= tokens_.size()) {
        throw UnexpectedEOFError();
    }

    TokenType current = tokens_[current_index_].type;

    if (expected == current) {
        ++current_index_;
        return true;
    }
    throw UnexpectedTokenError(
        "type " + std::to_string(static_cast<int>(expected)),
        tokens_[current_index_].value
    );
}

TokenType PDA::GetCurrentToken() const {
    if (current_index_ < tokens_.size()) {
        return tokens_[current_index_].type;
    }
    return TokenType::EndOfFile;
}

bool PDA::Parse() {
    if (tokens_.empty()) {
        throw EmptyInputError();
    }

    stack_.push(NonTerminal::E);
    current_index_ = 0;

    while (!stack_.empty()) {
        StackSymbol top = stack_.top();
        stack_.pop();

        if (IsTerminal(top)) {
            TokenType expected = std::get<TokenType>(top);
            if (!Match(expected)) {
                return false;
            }
        } else {
            NonTerminal nt = std::get<NonTerminal>(top);
            TokenType lookahead = GetCurrentToken();
            std::vector<StackSymbol> rule = SelectRule(nt, lookahead);

            if (rule.empty()) {
                if (nt == NonTerminal::E_ || nt == NonTerminal::T_) {
                    continue;
                }
                std::string token_val = (current_index_ < tokens_.size())
                    ? tokens_[current_index_].value
                    : "EOF";
                throw NoRuleFoundError(token_val);
            }

            for (auto it = rule.rbegin(); it != rule.rend(); ++it) {
                stack_.push(*it);
            }
        }
    }

    if (current_index_ < tokens_.size()) {
        throw UnprocessedTokensError(tokens_[current_index_].value);
    }

    return true;
}
         