#include "subexpression_finder.h"
#include "../util/subtree_utils.h"
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace {
struct AggregateInfo {
    size_t height = 0;
    size_t node_count = 0;
    std::vector<AST::NodePtr> nodes;
};

bool HasCoveredAncestor(const AST::NodePtr& node,
                        const std::unordered_set<const AST::Node*>& covered) {
    auto current = node->parent.lock();
    while (current) {
        if (covered.find(current.get()) != covered.end()) {
            return true;
        }
        current = current->parent.lock();
    }
    return false;
}
}

std::vector<RepeatedSubexpression> SubexpressionFinder::find(const AST& ast) const {
    std::vector<RepeatedSubexpression> result;
    auto root = ast.getRoot();
    if (!root) {
        return result;
    }

    std::unordered_map<std::string, AggregateInfo> aggregates;

    std::function<std::pair<size_t, size_t>(const AST::NodePtr&)> dfs =
        [&](const AST::NodePtr& node) -> std::pair<size_t, size_t> {
            if (!node) {
                return {0, 0};
            }
            auto left = dfs(node->left);
            auto right = dfs(node->right);
            std::string canonical = util::CanonicalForm(node);
            size_t height = 1 + std::max(left.first, right.first);
            size_t count = 1 + left.second + right.second;

            auto& info = aggregates[canonical];
            info.height = height;
            info.node_count = count;
            info.nodes.push_back(node);
            return {height, count};
        };

    dfs(root);

    std::vector<RepeatedSubexpression> candidates;
    candidates.reserve(aggregates.size());
    for (auto& [key, value] : aggregates) {
        if (value.nodes.size() < 2) {
            continue;
        }
        RepeatedSubexpression info;
        info.canonical = key;
        info.count = value.nodes.size();
        info.height = value.height;
        info.node_count = value.node_count;
        info.occurrences = value.nodes;
        candidates.push_back(std::move(info));
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const RepeatedSubexpression& lhs, const RepeatedSubexpression& rhs) {
                  if (lhs.height != rhs.height) {
                      return lhs.height > rhs.height;
                  }
                  if (lhs.count != rhs.count) {
                      return lhs.count > rhs.count;
                  }
                  if (lhs.node_count != rhs.node_count) {
                      return lhs.node_count > rhs.node_count;
                  }
                  return lhs.canonical < rhs.canonical;
              });

    std::unordered_set<const AST::Node*> covered;
    for (auto& candidate : candidates) {
        bool skip = true;
        for (auto& occurrence : candidate.occurrences) {
            if (!HasCoveredAncestor(occurrence, covered)) {
                skip = false;
                break;
            }
        }
        if (skip) {
            continue;
        }
        for (auto& occurrence : candidate.occurrences) {
            covered.insert(occurrence.get());
        }
        result.push_back(std::move(candidate));
    }

    return result;
}

