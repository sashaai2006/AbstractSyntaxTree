# 🌳 AST Visualizer

> A modern C++ Abstract Syntax Tree visualizer with interactive Qt GUI for mathematical expressions

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## ✨ Features

- 🎯 **Recursive Descent Parser** — Hand-written parser with proper operator precedence
- 🔍 **Subexpression Analysis** — Automatic detection of repeated and maximally closed subexpressions
- 🎨 **Interactive Visualization** — Real-time AST rendering with Qt6 Widgets
- ⚡ **Comprehensive Testing** — 20+ unit tests covering tokenizer, parser, and analysis
- 🛡️ **Robust Error Handling** — Detailed syntax error messages with position tracking
- 🧮 **Rich Expression Support** — Binary operators, unary functions, lambda expressions, parentheses

## 🚀 Quick Start

### Prerequisites

```bash
# macOS
brew install cmake qt@6

# Ubuntu/Debian
sudo apt install cmake qt6-base-dev

# Arch Linux
sudo pacman -S cmake qt6-base
```

### Build

```bash
git clone git@github.com:sashaai2006/AbstractSyntaxTree.git
cd AbstractSyntaxTree

cmake -S . -B build
cmake --build build
```

### Run

```bash
# Launch GUI
./build/lab2_app

# Run unit tests
./build/lab2_tests
```

## 📖 Usage

### GUI Application

1. Enter a mathematical expression in the input field
2. Click **"Build AST"**
3. View the generated tree and analysis results

**Supported syntax:**
- **Binary operators**: `+`, `-`, `*`, `/`, `^`
- **Unary functions**: `sin(x)`, `cos(x)`, `sqrt(x)`, `abs(x)`, `exp(x)`, `ln(x)`
- **Lambda expressions**: `lambda x. (x + 1)`
- **Parentheses**: `(2 + 3) * (x + y)`
- **Variables**: Single lowercase letters (`x`, `y`, `z`)
- **Constants**: Integers and decimals (`42`, `3.14`)

**Example expressions:**
```
sin(x) + cos(x)
(x + y) * (x + y) + x + y
lambda x. (x * x) + 1
2 ^ 3 ^ 4
```

### Programmatic API

```cpp
#include "parser/parser.h"
#include "analysis/subexpression_finder.h"

// Parse expression
Parser parser("2 * x + 3 * x");
AST ast = parser.buildAST();

// Find repeated subexpressions
SubexpressionFinder finder;
auto repeated = finder.find(ast);

for (const auto& item : repeated) {
    std::cout << item.canonical 
              << " appears " << item.count << " times\n";
}
```

## 🏗️ Architecture

### Project Structure

```
AbstractSyntaxTree/
├── parser/           # Lexer and parser implementation
│   ├── tokenizer.*   # Lexical analysis
│   ├── parser.*      # Recursive descent parser
│   └── pda.*         # Pushdown automaton (legacy validator)
├── ast/              # AST node definitions and traversals
├── analysis/         # Subexpression analysis algorithms
│   ├── subexpression_finder.*
│   └── msp_checker.*
├── util/             # Utility functions (canonical forms, etc.)
├── app/              # Qt GUI application
│   ├── main.cpp
│   └── astwidget.*   # AST rendering widget
└── tests/            # Comprehensive unit tests
```

### Key Components

#### 1. Tokenizer
Converts raw input into tokens with proper classification:
- Validates identifier length (single-letter variables only)
- Distinguishes unary/binary operators
- Recognizes keywords (`lambda`)

#### 2. Recursive Descent Parser
Implements grammar with correct precedence:
```
Expression  → Addition
Addition    → Multiplication (('+' | '-') Multiplication)*
Multiplication → Exponentiation (('*' | '/') Exponentiation)*
Exponentiation → Unary ('^' Exponentiation)?   # Right-associative
Unary       → UnaryOp Unary | Primary
Primary     → Number | ID | '(' Expression ')' | Lambda
```

#### 3. AST Representation
- **Nodes**: Store token, left/right children, weak parent pointer
- **Move semantics**: Efficient construction via `std::shared_ptr`
- **Traversals**: In-order (LCR), Post-order (LRC), Pre-order (CRL)

#### 4. Analysis Algorithms

**Repeated Subexpression Finder** (Section 2.10):
1. Collect all subtrees by height (1...n-1)
2. Filter out single-occurrence subtrees
3. Exclude nested subexpressions (keep maximal ones)
4. Return sorted by count/height

**Maximally Closed Subexpression Checker**:
- Identifies subexpressions without free variables
- Handles lambda binding correctly
- Excludes trivial leafs

## 🧪 Testing

```bash
# Run all tests
./build/lab2_tests

# Expected output:
# All tests passed successfully.
```

**Test coverage:**
- ✅ Tokenizer: whitespace handling, reset, unknown identifiers
- ✅ Parser: precedence, associativity, parentheses, lambdas
- ✅ AST: construction, traversals, parent links
- ✅ Utilities: canonical forms, height calculation, closedness
- ✅ Analysis: repeated patterns, commutativity, lambda scoping
- ✅ Error handling: syntax errors, mismatched parentheses

## 🎨 Screenshots

### Main Interface
*Expression input, AST visualization, and analysis panels*

### Error Handling
*Real-time syntax error detection with descriptive messages*

## 🛠️ Development

### Code Style

Following **Google C++ Style Guide**:
- `snake_case` for variables and functions
- `PascalCase` for classes
- Private members suffixed with `_`
- No comments (self-documenting code)
- Move semantics where applicable

### Build Targets

```bash
cmake --build build --target lab2_core    # Core library
cmake --build build --target lab2_tests   # Unit tests
cmake --build build --target lab2_app     # Qt application
```

### Adding New Features

1. Extend grammar in `parser/parser.cpp`
2. Update AST node types if needed
3. Add analysis in `analysis/` directory
4. Write comprehensive tests in `tests/run_tests.cpp`
5. Update UI in `app/astwidget.cpp`

## 📚 Algorithm Reference

Based on academic specification (Section 2.10):
- **Input**: Mathematical expression as string
- **Output**: Sorted list of `(subexpression, count)` pairs
- **Complexity**: O(n²) worst case, O(n log n) average
- **Method**: Bottom-up DFS with canonical form hashing

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing`)
3. Follow existing code style
4. Add tests for new functionality
5. Submit a pull request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Sasha**
- GitHub: [@sashaai2006](https://github.com/sashaai2006)
- Repository: [AbstractSyntaxTree](https://github.com/sashaai2006/AbstractSyntaxTree)

## 🙏 Acknowledgments

- Qt Framework for excellent GUI toolkit
- Google C++ Style Guide for code conventions
- Academic course material for algorithm specifications

---

⭐ **Star this repo if you find it useful!** ⭐
