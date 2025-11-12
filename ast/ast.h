#pragma once

#include "../parser/tokenizer.h"
#include <memory>
#include <string>
#include <vector>

class AST {
  public:
    struct Node {
        Token token;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        std::weak_ptr<Node> parent;

        explicit Node(Token token_value);
        Node(Token token_value,
             std::shared_ptr<Node> left_child,
             std::shared_ptr<Node> right_child);

        bool isLeaf() const;
    };

    using NodePtr = std::shared_ptr<Node>;

  private:
    NodePtr root_;

    void LCRTraversalRec(const NodePtr& node, std::vector<NodePtr>& result) const;
    void LRCTraversalRec(const NodePtr& node, std::vector<NodePtr>& result) const;
    void CRLTraversalRec(const NodePtr& node, std::vector<NodePtr>& result) const;
    size_t height(const NodePtr& node) const;

  public:
    AST() = default;
    explicit AST(NodePtr root);

    void setRoot(NodePtr root);
    NodePtr getRoot() const;
    bool empty() const;

    std::vector<NodePtr> LCRTraversal() const;
    std::vector<NodePtr> LRCTraversal() const;
    std::vector<NodePtr> CRLTraversal() const;
    size_t height() const;

    static NodePtr createNode(Token token,
                              NodePtr left,
                              NodePtr right);
    static NodePtr createLeaf(Token token);
};
