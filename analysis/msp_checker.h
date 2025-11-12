#pragma once

#include "../ast/ast.h"
#include <vector>

class MSPChecker {
  public:
    std::vector<AST::NodePtr> FindMaximallyClosed(const AST& ast) const;
};

