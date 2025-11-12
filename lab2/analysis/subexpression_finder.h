#pragma once

#include "../ast/ast.h"
#include <string>
#include <vector>

struct RepeatedSubexpression {
    std::string canonical;
    size_t count = 0;
    size_t height = 0;
    size_t node_count = 0;
    std::vector<AST::NodePtr> occurrences;
};

class SubexpressionFinder {
  public:
    std::vector<RepeatedSubexpression> find(const AST& ast) const;
};

