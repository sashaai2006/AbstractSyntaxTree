#include "ast.h"
#include <algorithm>
#include <utility>

namespace {
void SetParent(const AST::NodePtr& node, const AST::NodePtr& parent) {
    if (node) {
        node->parent = parent;
    }
}
}

AST::Node::Node(Token token_value)
    : token(std::move(token_value)), left(nullptr), right(nullptr), parent() {}

AST::Node::Node(Token token_value,
                std::shared_ptr<Node> left_child,
                std::shared_ptr<Node> right_child)
    : token(std::move(token_value)),
      left(std::move(left_child)),
      right(std::move(right_child)),
      parent() {}

bool AST::Node::isLeaf() const {
    return !left && !right;
}

AST::AST(NodePtr root) : root_(std::move(root)) {
    if (root_) {
        root_->parent.reset();
    }
}

void AST::setRoot(NodePtr root) {
    root_ = std::move(root);
    if (root_) {
        root_->parent.reset();
    }
}

AST::NodePtr AST::getRoot() const {
    return root_;
}

bool AST::empty() const {
    return root_ == nullptr;
}

void AST::LCRTraversalRec(const NodePtr& node, std::vector<NodePtr>& out) const {
    if (!node) {
        return;
    }
    LCRTraversalRec(node->left, out);
    out.push_back(node);
    LCRTraversalRec(node->right, out);
}

void AST::LRCTraversalRec(const NodePtr& node, std::vector<NodePtr>& out) const {
    if (!node) {
        return;
    }
    LRCTraversalRec(node->left, out);
    LRCTraversalRec(node->right, out);
    out.push_back(node);
}

void AST::CRLTraversalRec(const NodePtr& node, std::vector<NodePtr>& out) const {
    if (!node) {
        return;
    }
    out.push_back(node);
    CRLTraversalRec(node->left, out);
    CRLTraversalRec(node->right, out);
}

std::vector<AST::NodePtr> AST::LCRTraversal() const {
    std::vector<NodePtr> out;
    if (!root_) {
        return out;
    }
    LCRTraversalRec(root_, out);
    return out;
}

std::vector<AST::NodePtr> AST::LRCTraversal() const {
    std::vector<NodePtr> out;
    if (!root_) {
        return out;
    }
    LRCTraversalRec(root_, out);
    return out;
}

std::vector<AST::NodePtr> AST::CRLTraversal() const {
    std::vector<NodePtr> out;
    if (!root_) {
        return out;
    }
    CRLTraversalRec(root_, out);
    return out;
}

size_t AST::height() const {
    return height(root_);
}

size_t AST::height(const NodePtr& node) const {
    if (!node) {
        return 0;
    }
    return 1 + std::max(height(node->left), height(node->right));
}

AST::NodePtr AST::createNode(Token token,
                             NodePtr left,
                             NodePtr right) {
    auto node = std::make_shared<Node>(std::move(token), std::move(left), std::move(right));
    SetParent(node->left, node);
    SetParent(node->right, node);
    return node;
}

AST::NodePtr AST::createLeaf(Token token) {
    return std::make_shared<Node>(std::move(token));
}
