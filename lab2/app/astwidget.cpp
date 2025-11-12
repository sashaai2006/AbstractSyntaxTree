#include "astwidget.h"
#include "../util/subtree_utils.h"
#include <QPainter>
#include <QPen>
#include <QtGlobal>
#include <algorithm>

namespace {
QString FormatNodeLabel(const AST::NodePtr& node) {
    if (!node) {
        return {};
    }

    switch (node->token.type) {
        case TokenType::UnaryOperator: {
            if (node->left) {
                std::string operand = util::CanonicalForm(node->left);
                return QString::fromStdString(node->token.value + "(" + operand + ")");
            }
            return QString::fromStdString(node->token.value);
        }
        case TokenType::Lambda: {
            std::string parameter = node->left ? util::CanonicalForm(node->left) : "";
            std::string body = node->right ? util::CanonicalForm(node->right) : "";
            return QString::fromStdString("lambda " + parameter + ". " + body);
        }
        default:
            return QString::fromStdString(node->token.value);
    }
}
}  // namespace

ASTWidget::ASTWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(600, 400);
}

void ASTWidget::setTree(const AST& ast) {
    root_ = ast.getRoot();
    BuildLayout();
    update();
}

void ASTWidget::clear() {
    root_.reset();
    positioned_nodes_.clear();
    edges_.clear();
    update();
}

void ASTWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), Qt::white);

    if (!root_) {
        painter.setPen(Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, tr("AST is empty"));
        return;
    }

    QPen edgePen(Qt::darkGray, 2);
    painter.setPen(edgePen);
    for (const auto& edge : edges_) {
        painter.drawLine(edge.first, edge.second);
    }

    painter.setPen(Qt::black);
    for (const auto& node : positioned_nodes_) {
        QRectF ellipse(node.position.x() - 20, node.position.y() - 20, 40, 40);
        painter.setBrush(Qt::lightGray);
        painter.drawEllipse(ellipse);
        painter.drawText(ellipse, Qt::AlignCenter, FormatNodeLabel(node.node));
    }
}

void ASTWidget::BuildLayout() {
    positioned_nodes_.clear();
    edges_.clear();
    column_ = 1.0;

    if (!root_) {
        return;
    }

    std::unordered_map<const AST::Node*, QPointF> position_map;
    AssignPositions(root_, 0, position_map);

    std::vector<AST::NodePtr> nodes;
    CollectVisibleNodes(root_, nodes);

    positioned_nodes_.reserve(nodes.size());
    for (const auto& node : nodes) {
        auto it = position_map.find(node.get());
        if (it != position_map.end()) {
            positioned_nodes_.push_back({node, it->second});
        }
    }

    for (const auto& entry : positioned_nodes_) {
        QPointF from = entry.position;
        if (ShouldRenderChild(entry.node, entry.node->left)) {
            auto it = position_map.find(entry.node->left.get());
            if (it != position_map.end()) {
                edges_.push_back({from, it->second});
            }
        }
        if (ShouldRenderChild(entry.node, entry.node->right)) {
            auto it = position_map.find(entry.node->right.get());
            if (it != position_map.end()) {
                edges_.push_back({from, it->second});
            }
        }
    }
}

void ASTWidget::AssignPositions(const AST::NodePtr& node, int depth,
                                std::unordered_map<const AST::Node*, QPointF>& map) {
    if (!node) {
        return;
    }

    if (ShouldRenderChild(node, node->left)) {
        AssignPositions(node->left, depth + 1, map);
    }

    double x = column_ * horizontal_spacing_;
    double y = depth * vertical_spacing_ + vertical_spacing_;
    map[node.get()] = QPointF(x, y);
    column_ += 1.0;

    if (ShouldRenderChild(node, node->right)) {
        AssignPositions(node->right, depth + 1, map);
    }
}

void ASTWidget::CollectVisibleNodes(const AST::NodePtr& node,
                                    std::vector<AST::NodePtr>& out) const {
    if (!node) {
        return;
    }
    if (ShouldRenderChild(node, node->left)) {
        CollectVisibleNodes(node->left, out);
    }
    out.push_back(node);
    if (ShouldRenderChild(node, node->right)) {
        CollectVisibleNodes(node->right, out);
    }
}

bool ASTWidget::ShouldRenderChild(const AST::NodePtr& parent,
                                  const AST::NodePtr& child) {
    if (!child) {
        return false;
    }
    if (parent && parent->token.type == TokenType::UnaryOperator && child->isLeaf()) {
        return false;
    }
    return true;
}

