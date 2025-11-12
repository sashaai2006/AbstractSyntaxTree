#include "../analysis/msp_checker.h"
#include "../analysis/subexpression_finder.h"
#include "../parser/parser.h"
#include "../util/subtree_utils.h"
#include "astwidget.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>
#include <string>

class MainWindow : public QWidget {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget* parent = nullptr)
        : QWidget(parent),
          input_(new QLineEdit(this)),
          build_button_(new QPushButton(tr("Build AST"), this)),
          status_label_(new QLabel(this)),
          ast_widget_(new ASTWidget(this)),
          repeated_list_(new QListWidget(this)),
          msp_list_(new QListWidget(this)) {
        auto* main_layout = new QVBoxLayout(this);

        auto* control_layout = new QHBoxLayout();
        input_->setPlaceholderText(tr("Enter expression, e.g., (x + y) * (x + y)"));
        control_layout->addWidget(input_);
        control_layout->addWidget(build_button_);

        main_layout->addLayout(control_layout);
        main_layout->addWidget(status_label_);
        main_layout->addWidget(ast_widget_, 1);

        auto* lists_layout = new QHBoxLayout();
        auto* repeated_layout = new QVBoxLayout();
        repeated_layout->addWidget(new QLabel(tr("Repeated subexpressions:"), this));
        repeated_layout->addWidget(repeated_list_);

        auto* msp_layout = new QVBoxLayout();
        msp_layout->addWidget(new QLabel(tr("Maximally closed subexpressions:"), this));
        msp_layout->addWidget(msp_list_);

        lists_layout->addLayout(repeated_layout);
        lists_layout->addLayout(msp_layout);
        main_layout->addLayout(lists_layout);

        connect(build_button_, &QPushButton::clicked, this, &MainWindow::BuildAst);
    }

  private slots:
    void BuildAst() {
        const QString expression = input_->text().trimmed();
        if (expression.isEmpty()) {
            status_label_->setText(tr("Введите выражение"));
            ast_widget_->clear();
            repeated_list_->clear();
            msp_list_->clear();
            return;
        }

        try {
            Parser parser(expression.toStdString());
            AST ast = parser.buildAST();
            ast_widget_->setTree(ast);

            SubexpressionFinder finder;
            auto repeated = finder.find(ast);

            repeated_list_->clear();
            for (const auto& item : repeated) {
            if (item.occurrences.empty()) {
                continue;
            }
            TokenType token_type = item.occurrences.front()->token.type;
            if (token_type == TokenType::ID || token_type == TokenType::Number) {
                continue;
            }
                QString text =
                    QString::fromStdString(item.canonical + " -> count: " + std::to_string(item.count));
                repeated_list_->addItem(text);
            }

            MSPChecker checker;
            auto closed = checker.FindMaximallyClosed(ast);
            msp_list_->clear();
            for (const auto& node : closed) {
                QString text = QString::fromStdString(util::CanonicalForm(node));
                msp_list_->addItem(text);
            }

            status_label_->setText(tr("Построено успешно"));
        } catch (const ParserException& ex) {
            status_label_->setText(QString::fromStdString(ex.what()));
            ast_widget_->clear();
            repeated_list_->clear();
            msp_list_->clear();
        } catch (const std::exception& ex) {
            status_label_->setText(QString::fromStdString(ex.what()));
            ast_widget_->clear();
            repeated_list_->clear();
            msp_list_->clear();
        }
    }

  private:
    QLineEdit* input_;
    QPushButton* build_button_;
    QLabel* status_label_;
    ASTWidget* ast_widget_;
    QListWidget* repeated_list_;
    QListWidget* msp_list_;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(900, 600);
    window.setWindowTitle(QObject::tr("AST Visualizer"));
    window.show();
    return app.exec();
}

#include "main.moc"

