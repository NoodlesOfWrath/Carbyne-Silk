# Carbyne-Silk
A very simple C transpiler

# Overview
This C transpiler converts a custom scripting language (CBS) into C code. It uses a set of predefined replacements and syntax rules to map CBS code to equivalent C code.

## Features

* Tokenization: Converts CBS code into a sequence of tokens.

* Replacement Mapping: Defines a set of replacements for common CBS constructs.

* Syntax Matching: Matches CBS syntax against predefined C syntax patterns.

* Code Generation: Generates equivalent C code based on matched syntax.

# CBS Code
```python
for i in range(5) {
    print(i)
}
```
# Transpiled C Code
```c
for (int i = 0; i < 5; i++) {
    printf("%d\n", i);
}
```
