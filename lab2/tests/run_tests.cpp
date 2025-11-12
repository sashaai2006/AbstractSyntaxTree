#include "../analysis/msp_checker.h"
#include "../analysis/subexpression_finder.h"
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../parser/parser_exceptions.h"
#include "../parser/tokenizer.h"
#include "../util/subtree_utils.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

template <typename Exception, typename Callable>
void ExpectThrows(Callable&& callable) {
    bool threw = false;
    try {
        callable();
    } catch (const Exception&) {
        threw = true;
    }
    assert(threw);
}

void TestTokenizerBasicArithmetic() {
    Tokenizer tokenizer("2 * x + 3");
    auto tokens = tokenizer.tokenizeAll();
    assert(tokens.size() == 6);
    assert(tokens[0].type == TokenType::Number && tokens[0].value == "2");
    assert(tokens[1].type == TokenType::BinaryOperator && tokens[1].value == "*");
    assert(tokens[2].type == TokenType::ID && tokens[2].value == "x");
    assert(tokens[3].type == TokenType::BinaryOperator && tokens[3].value == "+");
    assert(tokens[4].type == TokenType::Number && tokens[4].value == "3");
    assert(tokens[5].type == TokenType::EndOfFile);
}

void TestTokenizerWhitespaceAndReset() {
    Tokenizer tokenizer("  \n\t x  +  4.5  ");
    Token first = tokenizer.nextToken();
    assert(first.type == TokenType::ID && first.value == "x");
    tokenizer.reset();
    Token again = tokenizer.nextToken();
    assert(again.type == TokenType::ID && again.value == "x");

    auto tokens = tokenizer.tokenizeAll();
    assert(tokens.size() == 4);
    assert(tokens[2].type == TokenType::Number && tokens[2].value == "4.5");
}

void TestTokenizerUnaryFunctionsAndLambda() {
    Tokenizer tokenizer("lambda.x + cos(y)");
    auto tokens = tokenizer.tokenizeAll();
    std::vector<TokenType> expected = {
        TokenType::Lambda,
        TokenType::Dot,
        TokenType::ID,
        TokenType::BinaryOperator,
        TokenType::UnaryOperator,
        TokenType::OpenScope,
        TokenType::ID,
        TokenType::CloseScope,
        TokenType::EndOfFile
    };
    assert(tokens.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        assert(tokens[i].type == expected[i]);
    }
}

void TestTokenizerRejectsUnknownIdentifiers() {
    ExpectThrows<SyntaxError>([] {
        Tokenizer tokenizer("vavava + 1");
        tokenizer.tokenizeAll();
    });
}

void TestParserStructureAndPrecedence() {
    Parser parser("2 + 3 * 4");
    AST ast = parser.buildAST();
    auto root = ast.getRoot();
    assert(root->token.value == "+");
    assert(root->left->token.value == "2");
    assert(root->right->token.value == "*");
    assert(root->right->left->token.value == "3");
    assert(root->right->right->token.value == "4");
}

void TestParserExponentRightAssociative() {
    Parser parser("2 ^ 3 ^ 2");
    AST ast = parser.buildAST();
    auto root = ast.getRoot();
    assert(root->token.value == "^");
    assert(root->left->token.value == "2");
    assert(root->right->token.value == "^");
    assert(root->right->left->token.value == "3");
    assert(root->right->right->token.value == "2");
}

void TestParserUnaryOperators() {
    Parser parser("-x + +y");
    AST ast = parser.buildAST();
    auto root = ast.getRoot();
    assert(root->token.value == "+");
    auto left = root->left;
    assert(left->token.value == "-");
    assert(left->left->token.value == "0");
    assert(left->right->token.value == "x");
    auto right = root->right;
    assert(right->token.value == "+");
    assert(right->left->token.value == "0");
    assert(right->right->token.value == "y");
}

void TestParserUnaryFunctionCall() {
    Parser parser("cos(x) + sin(y)");
    AST ast = parser.buildAST();
    auto root = ast.getRoot();
    assert(root->token.value == "+");
    auto left = root->left;
    assert(left->token.type == TokenType::UnaryOperator);
    assert(left->token.value == "cos");
    assert(left->left);
    assert(left->left->token.type == TokenType::ID);
    assert(left->left->token.value == "x");
    auto right = root->right;
    assert(right->token.type == TokenType::UnaryOperator);
    assert(right->token.value == "sin");
    assert(right->left);
    assert(right->left->token.type == TokenType::ID);
    assert(right->left->token.value == "y");
}

void TestParserRejectsUnaryFunctionWithoutParentheses() {
    ExpectThrows<ParserException>([] {
        Parser parser("cos x + 1");
        parser.buildAST();
    });
}

void TestParserLambdaExpression() {
    Parser parser("lambda x. (x * x) + 1");
    AST ast = parser.buildAST();
    auto root = ast.getRoot();
    assert(root->token.type == TokenType::Lambda);
    assert(root->left->token.type == TokenType::ID);
    assert(root->left->token.value == "x");
    auto body = root->right;
    assert(body->token.type == TokenType::BinaryOperator && body->token.value == "+");
    assert(body->right->token.type == TokenType::Number && body->right->token.value == "1");
    auto mult = body->left;
    assert(mult->token.type == TokenType::BinaryOperator && mult->token.value == "*");
    assert(mult->left->token.value == "x");
    assert(mult->right->token.value == "x");
}

void TestParserRejectsUnknownUnary() {
    ExpectThrows<ParserException>([] {
        Parser parser("foo(x)");
        parser.buildAST();
    });
}

void TestParserRejectsMismatchedParentheses() {
    ExpectThrows<ParserException>([] {
        Parser parser("(x + 1");
        parser.buildAST();
    });
}

void TestASTLeafAndNodeConstruction() {
    auto left = AST::createLeaf(Token{TokenType::ID, "a"});
    auto right = AST::createLeaf(Token{TokenType::ID, "b"});
    auto parent = AST::createNode(Token{TokenType::BinaryOperator, "+"}, left, right);
    AST ast(parent);
    assert(!ast.empty());
    assert(ast.getRoot() == parent);
    assert(left->parent.lock() == parent);
    assert(right->parent.lock() == parent);
    assert(left->isLeaf());
    assert(!parent->isLeaf());
    assert(ast.height() == 2);
}

void TestASTTraversals() {
    auto a = AST::createLeaf(Token{TokenType::ID, "a"});
    auto b = AST::createLeaf(Token{TokenType::ID, "b"});
    auto c = AST::createLeaf(Token{TokenType::ID, "c"});
    auto plus = AST::createNode(Token{TokenType::BinaryOperator, "+"}, a, b);
    auto root = AST::createNode(Token{TokenType::BinaryOperator, "*"}, plus, c);
    AST ast(root);

    auto inorder = ast.LCRTraversal();
    std::vector<std::string> inorder_values;
    for (const auto& node : inorder) {
        inorder_values.push_back(node->token.value);
    }
    std::vector<std::string> expected = {"a", "+", "b", "*", "c"};
    assert(inorder_values == expected);

    auto postorder = ast.LRCTraversal();
    std::vector<std::string> post_values;
    for (const auto& node : postorder) {
        post_values.push_back(node->token.value);
    }
    std::vector<std::string> expected_post = {"a", "b", "+", "c", "*"};
    assert(post_values == expected_post);

    auto preorder = ast.CRLTraversal();
    std::vector<std::string> pre_values;
    for (const auto& node : preorder) {
        pre_values.push_back(node->token.value);
    }
    std::vector<std::string> expected_pre = {"*", "+", "a", "b", "c"};
    assert(pre_values == expected_pre);
}

void TestASTSetRootResetsParent() {
    AST ast;
    auto node = AST::createLeaf(Token{TokenType::Number, "1"});
    ast.setRoot(node);
    assert(ast.getRoot() == node);
    assert(node->parent.expired());
    ast.setRoot(nullptr);
    assert(ast.empty());
}

void TestUtilCanonicalForm() {
    Parser parser("(a + b) + (b + a)");
    AST ast = parser.buildAST();
    auto left = ast.getRoot()->left;
    auto right = ast.getRoot()->right;
    assert(util::CanonicalForm(left) == util::CanonicalForm(right));
    assert(util::CanonicalForm(ast.getRoot()) == "+(+(a,b),+(a,b))");
}

void TestUtilHeightAndNodeCount() {
    Parser parser("((a + b) * c) + d");
    AST ast = parser.buildAST();
    assert(util::Height(ast.getRoot()) == ast.height());
    assert(util::NodeCount(ast.getRoot()) == ast.LCRTraversal().size());
}

void TestUtilIsClosedSubtree() {
    Parser parser("lambda x. lambda y. x + y");
    AST ast = parser.buildAST();
    auto root = ast.getRoot();
    assert(util::IsClosedSubtree(root));
    auto inner_lambda = root->left->right;
    assert(util::IsClosedSubtree(inner_lambda));

    Parser parser2("lambda x. y + 1");
    AST ast2 = parser2.buildAST();
    assert(!util::IsClosedSubtree(ast2.getRoot()));
}

void TestUtilCollectNodesPreOrder() {
    Parser parser("a + b * c");
    AST ast = parser.buildAST();
    std::vector<AST::NodePtr> nodes;
    util::CollectNodesPreOrder(ast.getRoot(), nodes);
    std::vector<std::string> values;
    for (const auto& node : nodes) {
        values.push_back(node->token.value);
    }
    std::vector<std::string> expected = {"+", "a", "*", "b", "c"};
    assert(values == expected);
}

void TestSubexpressionFinderDetectsRepeats() {
    Parser parser("(a + b) * (a + b) + (a + b)");
    AST ast = parser.buildAST();
    SubexpressionFinder finder;
    auto repeated = finder.find(ast);
    assert(!repeated.empty());
    assert(repeated.front().canonical == "+(a,b)");
    assert(repeated.front().count == 3);
}

void TestSubexpressionFinderRespectsCommutativity() {
    Parser parser("(a + b) + (b + a) + (a + b)");
    AST ast = parser.buildAST();
    SubexpressionFinder finder;
    auto repeated = finder.find(ast);
    bool found = false;
    for (const auto& item : repeated) {
        if (item.canonical == "+(a,b)") {
            found = true;
            assert(item.count == 3);
        }
    }
    assert(found);
}

void TestMSPCheckerOnLambdaAndConstants() {
    Parser parser("lambda x. (x + 5) + (lambda y. y) + 7");
    AST ast = parser.buildAST();
    MSPChecker checker;
    auto closed = checker.FindMaximallyClosed(ast);
    int lambda_count = 0;
    for (const auto& node : closed) {
        if (node->token.type == TokenType::Lambda) {
            ++lambda_count;
        }
    }
    assert(lambda_count >= 1);
}

void TestMSPCheckerOnStandaloneConstant() {
    Parser parser("42");
    AST ast = parser.buildAST();
    MSPChecker checker;
    auto closed = checker.FindMaximallyClosed(ast);
    assert(closed.size() == 1);
    assert(closed.front()->token.type == TokenType::Number);
    assert(closed.front()->token.value == "42");
}

void TestMSPCheckerSkipsNonClosed() {
    Parser parser("lambda x. y + 2");
    AST ast = parser.buildAST();
    MSPChecker checker;
    auto closed = checker.FindMaximallyClosed(ast);
    for (const auto& node : closed) {
        assert(node->token.type != TokenType::Lambda);
    }
}

void TestParserAndTokenizerErrorPropagation() {
    ExpectThrows<ParserException>([] {
        Parser parser("1 + (2 * 3");
        parser.buildAST();
    });

    ExpectThrows<ParserException>([] {
        Parser parser("lambda x. foo");
        parser.buildAST();
    });
}

}  // namespace

int main() {
    TestTokenizerBasicArithmetic();
    TestTokenizerWhitespaceAndReset();
    TestTokenizerUnaryFunctionsAndLambda();
    TestTokenizerRejectsUnknownIdentifiers();

    TestParserStructureAndPrecedence();
    TestParserExponentRightAssociative();
    TestParserUnaryOperators();
    TestParserUnaryFunctionCall();
    TestParserLambdaExpression();
    TestParserRejectsUnknownUnary();
    TestParserRejectsUnaryFunctionWithoutParentheses();
    TestParserRejectsMismatchedParentheses();
    TestParserAndTokenizerErrorPropagation();

    TestASTLeafAndNodeConstruction();
    TestASTTraversals();
    TestASTSetRootResetsParent();

    TestUtilCanonicalForm();
    TestUtilHeightAndNodeCount();
    TestUtilIsClosedSubtree();
    TestUtilCollectNodesPreOrder();

    TestSubexpressionFinderDetectsRepeats();
    TestSubexpressionFinderRespectsCommutativity();

    TestMSPCheckerOnLambdaAndConstants();
    TestMSPCheckerSkipsNonClosed();
    TestMSPCheckerOnStandaloneConstant();

    std::cout << "All tests passed successfully.\n";
    return 0;
}

