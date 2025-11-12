#pragma once

#include "../ast/ast.h"
#include <QPointF>
#include <QWidget>
#include <unordered_map>
#include <vector>

class ASTWidget : public QWidget {
    Q_OBJECT
  public:
    explicit ASTWidget(QWidget* parent = nullptr);

    void setTree(const AST& ast);
    void clear();

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    AST::NodePtr root_;

    struct PositionedNode {
        AST::NodePtr node;
        QPointF position;
    };

    std::vector<PositionedNode> positioned_nodes_;
    std::vector<std::pair<QPointF, QPointF>> edges_;
    double horizontal_spacing_ = 80.0;
    double vertical_spacing_ = 80.0;
    double column_ = 0.0;

    void BuildLayout();
    void AssignPositions(const AST::NodePtr& node, int depth,
                         std::unordered_map<const AST::Node*, QPointF>& map);
    void CollectVisibleNodes(const AST::NodePtr& node,
                             std::vector<AST::NodePtr>& out) const;
    static bool ShouldRenderChild(const AST::NodePtr& parent,
                                  const AST::NodePtr& child);
};

