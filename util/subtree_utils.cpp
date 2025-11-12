#include "subtree_utils.h"
#include <algorithm>
#include <sstream>
#include <utility>

namespace util {

namespace {
std::string CanonicalBinary(const std::string& op,
                            const std::string& left,
                            const std::string& right) {
    std::ostringstream oss;
    oss << op << "(" << left << "," << right << ")";
    return oss.str();
}
}

std::string CanonicalForm(const AST::NodePtr& node) {
    if (!node) {
        return "";
    }

    const Token& token = node->token;
    switch (token.type) {
        case TokenType::Number:
        case TokenType::ID:
            return token.value;
        case TokenType::UnaryOperator: {
            std::string child = CanonicalForm(node->left);
            return token.value + "(" + child + ")";
        }
        case TokenType::BinaryOperator: {
            std::string left = CanonicalForm(node->left);
            std::string right = CanonicalForm(node->right);
            if (token.value == "+" || token.value == "*") {
                if (left > right) {
                    std::swap(left, right);
                }
            }
            return CanonicalBinary(token.value, left, right);
        }
        case TokenType::Lambda: {
            std::string parameter = node->left ? CanonicalForm(node->left) : "";
            std::string body = CanonicalForm(node->right);
            return "lambda(" + parameter + "." + body + ")";
        }
        default:
            return token.value;
    }
}

size_t Height(const AST::NodePtr& node) {
    if (!node) {
        return 0;
    }
    return 1 + std::max(Height(node->left), Height(node->right));
}

size_t NodeCount(const AST::NodePtr& node) {
    if (!node) {
        return 0;
    }
    return 1 + NodeCount(node->left) + NodeCount(node->right);
}

bool IsClosedSubtree(const AST::NodePtr& node) {
    return IsClosedSubtree(node, {});
}

bool IsClosedSubtree(const AST::NodePtr& node,
                     std::unordered_set<std::string> bound_identifiers) {
    if (!node) {
        return true;
    }
    const Token& token = node->token;
    switch (token.type) {
        case TokenType::Number:
            return true;
        case TokenType::ID:
            return bound_identifiers.find(token.value) != bound_identifiers.end();
        case TokenType::UnaryOperator:
            return IsClosedSubtree(node->left, std::move(bound_identifiers));
        case TokenType::BinaryOperator: {
            auto left_bound = bound_identifiers;
            bool left_closed = IsClosedSubtree(node->left, std::move(left_bound));
            bool right_closed = IsClosedSubtree(node->right, std::move(bound_identifiers));
            return left_closed && right_closed;
        }
        case TokenType::Lambda: {
            if (node->left && node->left->token.type == TokenType::ID) {
                bound_identifiers.insert(node->left->token.value);
            }
            if (node->left) {
                IsClosedSubtree(node->left, bound_identifiers);
            }
            return IsClosedSubtree(node->right, std::move(bound_identifiers));
        }
        default:
            return false;
    }
}

void CollectNodesPreOrder(const AST::NodePtr& node,
                          std::vector<AST::NodePtr>& out) {
    if (!node) {
        return;
    }
    out.push_back(node);
    CollectNodesPreOrder(node->left, out);
    CollectNodesPreOrder(node->right, out);
}

}
