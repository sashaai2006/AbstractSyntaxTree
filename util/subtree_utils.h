#pragma once

#include "../ast/ast.h"
#include <string>
#include <unordered_set>
#include <vector>

namespace util {

std::string CanonicalForm(const AST::NodePtr& node);
size_t Height(const AST::NodePtr& node);
size_t NodeCount(const AST::NodePtr& node);
bool IsClosedSubtree(const AST::NodePtr& node);
bool IsClosedSubtree(const AST::NodePtr& node,
                     std::unordered_set<std::string> bound_identifiers);
void CollectNodesPreOrder(const AST::NodePtr& node,
                          std::vector<AST::NodePtr>& out);

}

