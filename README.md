# 8-bit CPU Emulator and Assembler

This project implements a small custom 8-bit CPU, a matching assembler, a memory model, an instruction executor, and simple command-line front ends for assembling and running programs [1][2][3][4]. The codebase is organized so that assembly source can either be assembled separately into a binary file or assembled on the fly and executed directly by the emulator [1][2].

## What the project contains

At a high level, the project consists of five main parts [3][4][5][6][7]:

- A CPU model that stores registers, flags, the program counter, stack pointer, and exposes `step()` and `run()` methods [3][8][9]
- A memory module that provides a flat 64 KiB address space with byte-wise reads and writes [7][10]
- An instruction set definition and executor implementing arithmetic, logic, control flow, stack, load/store, and output instructions [11][6]
- An assembler that parses text assembly, resolves labels, supports directives like `.ORG` and `.DB`, and emits machine code bytes [4][12]
- Loader utilities and command-line entry points for loading binaries or executing assembled source files [5][1][2]

## Project structure

The source files correspond closely to the machine components:

- `Assembler.hpp` / `Assembler.cpp` — assembly parsing and bytecode generation [12][4]
- `AssemblerMain.cpp` — standalone assembler executable that reads assembly source and writes a binary output file [2]
- `CPU.hpp` / `CPU.cpp` — CPU state transitions, fetch helpers, stack handling, and execution loop [3][8]
- `Instructions.hpp` / `Instructions.cpp` — opcode definitions and instruction execution logic [11][6]
- `Loader.hpp` / `Loader.cpp` — loading raw binaries or assembled byte vectors into memory and initializing CPU state [13][5]
- `Memory.hpp` / `Memory.cpp` — 64 KiB RAM abstraction [7][10]
- `Registers.hpp` — register file, flags, and global CPU state representation [9]
- `main.cpp` — emulator executable that accepts either assembly source or binary input and runs it [1]

## CPU model

The CPU exposes eight 8-bit general-purpose registers stored in `CPUState::R[8]`, along with a 16-bit program counter `PC`, a 16-bit stack pointer `SP`, and four status flags: zero (`Z`), carry (`C`), overflow (`V`), and sign (`S`) [9]. The `CPU` class owns both the current machine state and the full memory object, and it provides helpers for fetching instruction bytes, decoding register operands, manipulating the stack, and updating arithmetic or logic flags [3][8].

Execution proceeds one instruction at a time through `step()`, which fetches an opcode from memory at the current `PC`, dispatches to the instruction executor, and stops once the machine halts or execution fails [8][6]. The `run()` method repeatedly calls `step()` until the program halts or an optional cycle limit is reached [8].

## Memory model

Memory is implemented as a fixed-size `std::array` of 65536 bytes, which gives the emulator a flat 16-bit address space from `0x0000` to `0xFFFF` [7]. Reads and writes are simple direct indexed accesses through `Memory::read()` and `Memory::write()` [10].

The loader initializes the machine by copying either a binary file or an already assembled byte vector into memory beginning at a chosen origin address [5]. After loading, it sets `PC` to the origin, initializes `SP` to `0xFFFF`, and clears the halted state so execution can begin [5].

## Instruction set

The project defines an explicit opcode enum covering no-op, data movement, arithmetic and logic, comparisons, control flow, subroutines, stack operations, output, and halt [11]. The currently defined instruction mnemonics are `NOP`, `MOV`, `LDI`, `LDR`, `STR`, `ADD`, `SUB`, `AND`, `OR`, `XOR`, `NOT`, `SHL`, `SHR`, `CMP`, `JMP`, `JZ`, `JNZ`, `JC`, `JN`, `JV`, `CALL`, `RET`, `PUSH`, `POP`, `OUT`, and `HLT` [11][6].

The executor in `Instructions.cpp` implements these operations by decoding operands from memory and updating registers, flags, memory, or control flow accordingly [6]. For example, arithmetic instructions call `setArithmeticFlags()`, logical instructions call `setLogicFlags()`, subroutines use 16-bit stack push/pop helpers, and `OUT` prints a register as a character to standard output [8][6].

One notable design choice is that `LDR` and `STR` treat a register pair as a 16-bit address by combining adjacent registers, with the selected register contributing the high byte and the next register contributing the low byte [6]. Conditional jumps use the CPU flags, so `JZ`, `JNZ`, `JC`, `JN`, and `JV` depend on flag values produced by prior arithmetic or comparison instructions [6][9].

## Assembler

The assembler reads assembly source from a file path and returns a `std::vector<uint8_t>` containing the assembled program bytes [12][4]. It uses a two-pass algorithm: the first pass strips comments, trims lines, collects label addresses, processes `.ORG`, and counts bytes for `.DB` and instructions; the second pass emits the actual machine code using resolved labels and parsed operands [4].

The parser accepts register names `R0` through `R7` and aliases `A` through `H` [4]. It also supports decimal, hexadecimal (`0x...`), binary (`0b...`), character literals, and several escaped character forms such as `\n`, `\t`, and `\0` [4].

The assembler understands at least two directives visible in the implementation:

- `.ORG value` — set the current assembly address [4]
- `.DB value1, value2, ...` — emit raw bytes [4]

Lines may contain comments starting with `;`, and labels are written with a trailing `:` [4]. Mnemonics and labels are normalized to uppercase internally, so source is effectively case-insensitive [4].

## Command-line programs

The repository contains two separate entry points [1][2]:

### Emulator

`main.cpp` builds an emulator executable that accepts a single input file path [1]. If the file name ends in `.txt` or `.asm`, the program assembles it in memory and immediately loads the resulting bytes into the CPU; otherwise it treats the input as a raw binary file and loads it directly [1].

The usage string in the source indicates a single-argument invocation pattern, even though the exact placeholder text in the snippet is truncated [1]. Operationally, it behaves like this:

```bash
./emulator program.asm
./emulator program.txt
./emulator program.bin
```

### Standalone assembler

`AssemblerMain.cpp` builds a separate assembler executable that expects exactly two arguments: an input assembly file and an output binary file [2]. It assembles the source into bytes and writes those bytes directly to the output file in binary mode [2].

Usage behavior is therefore equivalent to:

```bash
./assembler program.asm program.bin
```

If assembly fails or the output file cannot be opened, the program reports an error message and returns a nonzero exit status [2].

## Build instructions

The attached source files do not include a build system such as CMake or a Makefile, so the simplest way to build the project is to compile the translation units directly with a modern C++ compiler [12][2][4][3][8][11][6][13][5][10][1][9][7]. A compiler with C++17 support or newer is a sensible default because the code uses standard library containers, strings, streams, and fixed-width integer types extensively [4][7][3].

Example build commands with `g++`:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  main.cpp Assembler.cpp CPU.cpp Instructions.cpp Loader.cpp Memory.cpp \
  -o emulator

g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  AssemblerMain.cpp Assembler.cpp \
  -o assembler
```

If your actual filenames contain numeric suffixes such as `main-13.cpp` or `CPU-4.cpp`, either rename them to the include-matching names shown in the source or compile using the exact filenames present in your directory [1][8][2]. The includes in the code reference names like `CPU.hpp`, `Assembler.hpp`, and `Loader.hpp`, so consistent file naming matters for a successful build [1][2][3].

On Linux or macOS, build from the project root so the compiler can find the local headers in the same directory [1][2]. If needed, add `-I.` explicitly to the compiler command line [1][2].

## Running programs

There are two normal workflows for using the project [1][2]:

1. Assemble and run in one step through the emulator:

```bash
./emulator example.asm
```

This path is convenient during development because the emulator will call the assembler internally for `.asm` or `.txt` inputs [1].

2. Assemble first, then run the produced binary:

```bash
./assembler example.asm example.bin
./emulator example.bin
```

This path is useful if you want to inspect, save, or distribute the generated machine code [2][1].

## Example assembly program

A minimal program for this ISA can look like this:

```asm
.ORG 0x0000

start:
    LDI A, 'H'
    OUT A
    LDI A, 'i'
    OUT A
    LDI A, '\n'
    OUT A
    HLT
```

This program loads ASCII character values into register `A`, prints them using `OUT`, and terminates with `HLT` [4][6]. It is a simple sanity test for the assembler, loader, execution loop, and output instruction [4][6][1].

## Error handling

The codebase already contains several useful runtime and assembly-time checks [2][4][5]. The assembler reports issues such as unknown instructions, invalid registers, malformed literals, and out-of-range 8-bit immediates [4]. The loader rejects empty files and programs that would overflow the 16-bit address space when loaded at the selected origin [5].

The emulator and standalone assembler both catch `std::exception` and print readable error messages to standard error before returning a failure status [1][2]. This makes the tools practical to use from a shell or inside a larger build pipeline [1][2].
