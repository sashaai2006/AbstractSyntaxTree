#include "msp_checker.h"
#include "../util/subtree_utils.h"
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace {
bool EvaluateClosed(const AST::NodePtr& node,
                    std::unordered_set<std::string> bound,
                    std::unordered_map<const AST::Node*, bool>& memo) {
    if (!node) {
        return true;
    }

    auto memo_it = memo.find(node.get());
    if (memo_it != memo.end()) {
        return memo_it->second;
    }

    bool closed = false;
    const Token& token = node->token;

    switch (token.type) {
        case TokenType::Number:
            closed = true;
            break;
        case TokenType::ID:
            closed = bound.find(token.value) != bound.end();
            break;
        case TokenType::UnaryOperator:
            closed = EvaluateClosed(node->left, std::move(bound), memo);
            break;
        case TokenType::BinaryOperator: {
            auto left_bound = bound;
            bool left_closed = EvaluateClosed(node->left, std::move(left_bound), memo);
            bool right_closed = EvaluateClosed(node->right, std::move(bound), memo);
            closed = left_closed && right_closed;
            break;
        }
        case TokenType::Lambda: {
            if (node->left && node->left->token.type == TokenType::ID) {
                bound.insert(node->left->token.value);
                memo[node->left.get()] = true;
            }
            closed = EvaluateClosed(node->right, std::move(bound), memo);
            break;
        }
        default:
            closed = false;
            break;
    }

    memo[node.get()] = closed;
    return closed;
}
}

std::vector<AST::NodePtr> MSPChecker::FindMaximallyClosed(const AST& ast) const {
    std::vector<AST::NodePtr> result;
    auto root = ast.getRoot();
    if (!root) {
        return result;
    }

    std::unordered_map<const AST::Node*, bool> closed_memo;
    EvaluateClosed(root, {}, closed_memo);

    std::vector<AST::NodePtr> traversal;
    util::CollectNodesPreOrder(root, traversal);

    for (const auto& node : traversal) {
        auto closed_it = closed_memo.find(node.get());
        if (closed_it == closed_memo.end() || !closed_it->second) {
            continue;
        }
        auto parent = node->parent.lock();
        if (!parent) {
            result.push_back(node);
            continue;
        }
        auto parent_it = closed_memo.find(parent.get());
        if (parent_it == closed_memo.end() || !parent_it->second) {
            result.push_back(node);
        }
    }

    return result;
}

