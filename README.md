# symbolic-c

A symbolic computer algebra system (CAS) written in C, designed for embedded systems and custom hardware.  
Features a command-line interface and modular architecture for parsing, constructing, and simplifying mathematical expressions.

---

## Features

- **Tokenizes and Parses Expressions:**  
  Handles numeric, symbolic, and operator tokens, including implicit multiplication (e.g., `2x`, `3(x+1)`).
- **Builds Abstract Syntax Trees (ASTs):**  
  Utilizes a direct shunting yard algorithm for robust AST construction.
- **Algebraic Simplification:**  
  Implements arithmetic and algebraic properties (identity, zero, distributive, commutative, associative, powers, etc.).
- **Memory-Efficient:**  
  Uses custom token and node pools for memory management—ideal for embedded platforms.
- **Designed for Extensibility:**  
  Modular code structure allows for easy addition of new operators, functions, or simplification rules.
- **Hardware Integration:**  
  Targeting the Raspberry Pi Zero, with plans for a custom PCB and Arduino-based GUI for tactile input and display navigation.

---

## Getting Started

### Prerequisites

- **C Compiler:** GCC (Linux, Raspberry Pi), clang, or equivalent.
- **CMake:** For build configuration (optional, but recommended).
- **Hardware (Optional):**
  - Raspberry Pi Zero (or other ARM SBC)
  - Arduino (for GUI/Display, optional)
  - Custom PCB (in development)

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

> x^2 + 2*x + 1
(x + 1)^2

> 3(a + b)
3a + 3b
```

---

## Project Structure

- `src/`
  - `expression_t/` — Expression parsing, AST construction, and simplification
  - `token_t/` — Token types and pool management
  - `node_t/` — AST node types, pools, and tree logic
  - `stack_t/` — Simple stack implementation for parsing and evaluation
- `main.c` — CLI entry point (to be added or extended)
- `include/` — Header files

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
- [ ] CLI improvements (history, multi-line input)
- [ ] Add support for functions (`sin`, `cos`, etc.)
- [ ] Error handling and reporting
- [ ] Hardware GUI integration (Arduino)
- [ ] Custom PCB design and documentation

---

## Contributing

Pull requests and suggestions are welcome!  
If you have ideas for new features, bug reports, or want to help with hardware, open an issue or contact [atbev23](https://github.com/atbev23).

---

## License

MIT License.  
See [LICENSE](LICENSE) for details.

---

## Acknowledgments

- [Wikipedia: Computer Algebra Systems](https://en.wikipedia.org/wiki/Computer_algebra_system)
- [Shunting Yard Algorithm](https://en.wikipedia.org/wiki/Shunting_yard_algorithm)
