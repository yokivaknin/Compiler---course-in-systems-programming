# Project Name: Assembler
# Overview
This project is an assembler program written in C. It reads assembly language source files, processes them, and generates machine code. The generated machine code can be executed on a hypothetical computer architecture.
# Features
1. Supports various assembly language instructions.
2. Handles different addressing modes for operands.
3. Processes data and instruction sections.
4. Manages symbol tables.
5. Performs two-pass assembly: first pass for collecting information, second pass for generating code.
6. Error handling for invalid input and memory allocation issues.
7. Generates external files for references to external symbols.
# Usage
To use the assembler, follow these steps:

1. Compile the source code using a C compiler.
2. Run the executable with the assembly source file(s) as arguments.

Example:

$ gcc -o assembler main.c

$ ./assembler file1.as file2.as

# File Structure
assemblerUtils.h: Header file containing function prototypes and global variables.

macro.h: Header file for macro processing functions.

utils.h: Header file containing functions in large usage through the project. 

main.c: Main program file containing the entry point and logic for assembling files.

# Dependencies
Standard C libraries: stdio.h, ctype.h, math.h

Custom header files: assemblerUtils.h, macro.h, utils.h

# Contributors
yoki vaknin

