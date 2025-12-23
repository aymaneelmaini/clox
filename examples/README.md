# KLox Sample Programs

This directory contains example programs demonstrating various features of the KLox language.

## Valid Examples

### Basic Programs
- **fibonacci.lox** / **example_fibonacci.lox** - Recursive Fibonacci sequence calculator
- **loops.lox** - For and while loop examples
- **control_flow.lox** - If/else and control flow examples
- **operators.lox** - Arithmetic, comparison, and logical operators
- **functions.lox** - Function declaration and usage

### Advanced Features
- **closures.lox** - Demonstrates lexical scoping and closures
- **anonymous_functions.lox** - Lambda/anonymous function examples

### Object-Oriented Programming
- **classes.lox** - Classes and methods
- **inheritance.lox** - Class inheritance
- **super.lox** - Superclass method calls
- **static_methods.lox** - Static methods on classes

### Native Functions
- **native_lox.lox** - Using built-in clock() function
- **native_input.lox** - Using input() for user input
- **native_scan.lox** - Using scan() for reading input

### Complete Examples
- **example_quiz.lox** - Interactive quiz program
- **example_user_db.lox** - User database example

## Error Examples

The `errors/` directory contains programs that demonstrate the beautiful error reporting system:

### Parse Errors
- **parse_error.lox** - Missing expression after operator

### Runtime Errors
- **runtime_error_division_by_zero.lox** - Division by zero
- **runtime_error_undefined_variable.lox** - Accessing undefined variables
- **type_error.lox** - Type mismatch in operations

## Running Examples

To run a valid example:
```bash
java -jar build/libs/klox.jar samples/fibonacci.lox
```

To see the beautiful error reporting in action:
```bash
java -jar build/libs/klox.jar samples/errors/parse_error.lox
```

To start the interactive REPL:
```bash
java -jar build/libs/klox.jar
```

## Features Demonstrated

- ✓ Variables and scoping
- ✓ Functions (named and anonymous)
- ✓ Classes and inheritance
- ✓ Closures
- ✓ Loops (for, while)
- ✓ Conditionals (if/else)
- ✓ Operators (arithmetic, comparison, logical)
- ✓ Native functions (clock, println, scan, input)
- ✓ Static methods
- ✓ Super keyword for parent class access
- ✓ Beautiful Rust-style error reporting with source context
