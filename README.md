# symbolic-c

A symbolic computer algebra system (CAS) written in C, focused on building and simplifying algebraic expressions using a lightweight, statically-allocated architecture.  

It features a command-line interface and modular architecture for parsing, constructing, and simplifying mathematical expressions.

---

## Features

- **Tokenizes and Parses Expressions:**  
    - Handles numeric, symbolic, unary (only minus right now), and operator tokens, including implicit multiplication (e.g., `2x`, `3(x+1)`) with plans to expand to function tokens (e.g., `sin(x)`, `log(x)`).
- **Builds Abstract Syntax Trees (ASTs):**  
  - Utilizes a direct shunting yard algorithm for AST construction.
- **Expression Standardization:**
  - Normalizes the AST by rewriting subtraction, division, and unary negation into a consistent additive and multiplicative form, providing a canonical representation for further symbolic simplification.
- **Algebraic Simplification:**
  - Implements arithmetic and algebraic properties (identity, zero, distributive, commutative, associative, powers, etc.).
- **Memory-Efficient:**  
  - Uses custom token and node pools for memory management—ideal for constrained or embedded platforms.
- **Designed for Extensibility:**  
  - Modular code structure allows for easy addition (but not integration) of new operators, functions, or simplification rules.

---

## Getting Started

### Prerequisites

- **C Compiler:** GCC (Linux, Raspberry Pi), clang, or equivalent.
- **CMake:** For build configuration (optional, but recommended).

### Building the Project

Clone the repository:
```sh
git clone https://github.com/atbev23/symbolic-c.git
cd symbolic-c
```

Build using GCC (example):
```sh
gcc -o symbolic src/expression_t/*.c src/token_t/*.c src/node_t/*.c src/stack_t/*.c -lm
```

Or use CMake:
```sh
mkdir build
cd build
cmake ..
make
```

### Running

```sh
./symbolic
```

You’ll be greeted with a command-line interface for entering and evaluating expressions.

---

## Example Usage

```
> 2 + 2 * 2
6

> 3(x + y + 2)
3x + 3y + 6
```

---

## Project Structure

- `resources/`
  - `icon` — a semi-custom icon with source files, .svg, .png, and Windows .res file
- `src/`
  - `expression_t/` — Expression parsing, AST construction, and simplification
  - `token_t/` — Token types and pool management
  - `node_t/` — AST node types, pools, and tree logic
  - `stack_t/` — Simple stack implementation for parsing and evaluation
  - `queue_t/` — Circular queue implementation
- `main.c` — CLI entry point (to be added or extended)

---

## Hardware Integration Plans

- **Embedded Platform:**  
  Designed for the Raspberry Pi Zero for low-power, portable math computing.
- **Custom PCB:**  
  Planned to integrate Pi Zero, power supply, and user IO (buttons, encoders).

---

## Status & Roadmap

- [x] Expression parsing and tokenization
- [x] AST construction (shunting yard)
- [x] Algebraic simplification (commutative, associative, distributive, powers)
- [ ] Calculus (derivation, some integration)
- [ ] CLI improvements (history, multi-line input)
- [ ] Add support for functions (`sin`, `cos`, etc.)
- [ ] Error handling and reporting
- [ ] Custom PCB design and documentation

---

## Acknowledgments

- [Wikipedia: Computer Algebra Systems](https://en.wikipedia.org/wiki/Computer_algebra_system)
- [Shunting Yard Algorithm](https://en.wikipedia.org/wiki/Shunting_yard_algorithm)
- [Intrduction to Circular Queue](https://www.geeksforgeeks.org/dsa/introduction-to-circular-queue/)