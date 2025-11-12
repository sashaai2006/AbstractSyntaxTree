#pragma once
#include "tokenizer.h"
#include <stack>
#include <unordered_map>
#include <vector>
#include <variant>

enum class NonTerminal {
    E,
    E_,
    T,
    T_,
    F
};

using StackSymbol = std::variant<TokenType, NonTerminal>;

class PDA {
  private:
    std::stack<StackSymbol> stack_;
    std::unordered_map<NonTerminal, std::vector<std::vector<StackSymbol>>> grammar_;
    std::vector<Token> tokens_;
    size_t current_index_ = 0;

    bool IsTerminal(const StackSymbol& symbol) const;
    bool IsNonTerminal(const StackSymbol& symbol) const;
    std::vector<StackSymbol> SelectRule(NonTerminal non_terminal, TokenType lookahead);
    bool Match(TokenType expected);
    TokenType GetCurrentToken() const;
    void InitGrammar();

  public:
    explicit PDA(const std::vector<Token>& input_tokens);
    bool Parse();
};