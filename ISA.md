# ISA Documentation

This document describes the Instruction Set Architecture (ISA) implemented by this project, based on the instruction definitions, operand parsing rules, and assembler behavior present in `Assembler.cpp` [1]. An ISA defines the programmer-visible machine model, including registers, instruction formats, operands, control flow, and the rules used to write valid assembly programs [2][3].

## Overview

The assembler in this project reads assembly source code from a file, strips comments, trims whitespace, resolves labels, interprets directives such as `.ORG` and `.DB`, and emits a flat byte sequence representing the machine code program [1]. It uses a two-pass design so that labels can be referenced before they are defined, because pass one computes addresses and pass two emits the final encoded bytes [1].

The ISA exposed by the assembler is small and regular. Instructions are grouped by operand shape: no operand, one register, two registers, register plus immediate, and one 16-bit address operand [1]. This makes the instruction set easy to learn and straightforward to assemble by hand [1].

## Programmer model

The CPU provides eight general-purpose registers named `R0` through `R7` [1]. The assembler also accepts alias names `A` through `H`, mapped directly to `R0` through `R7`, so `A` is the same register as `R0`, `B` is the same as `R1`, and so on up to `H` as `R7` [1].

The assembler works with byte-sized instruction fields and 16-bit addresses [1]. Immediate values used by `LDI` must fit into 8 bits, while jump and call targets are encoded as 16-bit addresses split into high byte followed by low byte [1].

## Source format

Assembly source is line-oriented. Each line may contain labels, a directive, an instruction, a comment, or be empty [1]. Comments begin with `;`, and everything after that character on the same line is ignored by the assembler [1].

Whitespace at the beginning and end of tokens is ignored because the assembler trims input before interpreting it [1]. Mnemonics, labels, and register names are treated case-insensitively because the assembler normalizes tokens to uppercase before lookup [1].

### Labels

A label is written as an identifier followed by `:` and represents the current program counter value at that point in the source [1]. Labels are collected in the first pass, stored in uppercase form, and later reused when an instruction or directive needs an address or symbolic constant [1].

A line may contain more than one label before an instruction or directive, because the assembler repeatedly scans for `:` while processing a line in pass one [1]. This means several symbolic names can refer to the same address if needed [1].

Example:

```asm
start:
loop:
    NOP
```

In this example, both `start` and `loop` refer to the address of the `NOP` instruction [1].

## Constants and literals

The assembler accepts several literal forms through its shared value parser [1]. A value may be a label reference, a decimal integer, a hexadecimal integer beginning with `0x`, a binary integer beginning with `0b`, or a character literal in single quotes [1].

Supported character escapes include `\n`, `\r`, `\t`, `\0`, `\\`, and `\'` [1]. Invalid character or binary literals produce explicit assembler errors, which helps catch malformed source early [1].

Examples:

```asm
LDI A, 42
LDI B, 0x2A
LDI C, 0b00101010
.DB 'A', '\n', 0
JMP start
```

These are all valid according to the parsing rules visible in `parseValue` [1].

## Directives

The assembler supports at least two directives directly visible in the implementation: `.ORG` and `.DB` [1]. Directives are not CPU instructions; they control how the assembler places code and data in memory [3][1].

### .ORG

`.ORG value` sets the current program counter to `value` [1]. This allows code or data to be placed at a specific address in the output image [1].

Example:

```asm
.ORG 0x0100
start:
    HLT
```

Here, the label `start` is assigned address `0x0100` during assembly [1].

### .DB

`.DB value1, value2, ...` emits raw byte data into the output [1]. During pass one, the assembler advances the program counter by the number of operands in the `.DB` list, and during code emission each operand becomes one byte of data [1].

Example:

```asm
msg: .DB 'H', 'i', '\n', 0
```

This defines four consecutive bytes beginning at label `msg` [1].

## Register syntax

Registers can be written either as numbered registers or aliases [1]. Valid names are:

- `R0`, `R1`, `R2`, `R3`, `R4`, `R5`, `R6`, `R7`
- `A`, `B`, `C`, `D`, `E`, `F`, `G`, `H` [1]

Any other register token causes an `invalid register` error during assembly [1].

## Instruction formats

Each instruction starts with a one-byte opcode [1]. The remaining bytes depend on the instruction kind, which is determined from the assembler’s opcode table [1].

The defined operand classes are:

- `None`: opcode only, total size 1 byte [1]
- `OneReg`: opcode, register, total size 2 bytes [1]
- `TwoReg`: opcode, register1, register2, total size 3 bytes [1]
- `RegImm8`: opcode, register, 8-bit immediate, total size 3 bytes [1]
- `Addr16`: opcode, high address byte, low address byte, total size 3 bytes [1]

The assembler uses these sizes in pass one to assign correct addresses to labels before final code emission [1].

## Instruction reference

The following instruction set is defined directly in the `ops` table in `Assembler.cpp` [1]. The semantic descriptions below follow the instruction names and operand classes, while exact runtime effects such as flags and memory interactions are ultimately determined by the CPU implementation [1].

| Mnemonic | Opcode | Format | Operands | Description |
|---------|--------|--------|----------|-------------|
| `NOP` | `0x00` | None | none | No operation [1] |
| `MOV` | `0x10` | TwoReg | `dst, src` | Copy one register into another [1] |
| `LDI` | `0x11` | RegImm8 | `reg, imm8` | Load an 8-bit immediate into a register [1] |
| `LDR` | `0x12` | TwoReg | `dst, src` | Load through a register-based operand [1] |
| `STR` | `0x13` | TwoReg | `dst, src` | Store through a register-based operand [1] |
| `ADD` | `0x20` | TwoReg | `dst, src` | Add source register to destination register [1] |
| `SUB` | `0x21` | TwoReg | `dst, src` | Subtract source register from destination register [1] |
| `AND` | `0x22` | TwoReg | `dst, src` | Bitwise AND [1] |
| `OR` | `0x23` | TwoReg | `dst, src` | Bitwise OR [1] |
| `XOR` | `0x24` | TwoReg | `dst, src` | Bitwise XOR [1] |
| `NOT` | `0x25` | OneReg | `reg` | Bitwise NOT of one register [1] |
| `SHL` | `0x26` | OneReg | `reg` | Shift register left [1] |
| `SHR` | `0x27` | OneReg | `reg` | Shift register right [1] |
| `CMP` | `0x28` | TwoReg | `a, b` | Compare two registers, typically affecting flags [1] |
| `JMP` | `0x30` | Addr16 | `addr` | Unconditional jump [1] |
| `JZ` | `0x31` | Addr16 | `addr` | Jump if zero condition is true [1] |
| `JNZ` | `0x32` | Addr16 | `addr` | Jump if zero condition is false [1] |
| `JC` | `0x33` | Addr16 | `addr` | Jump if carry condition is true [1] |
| `JN` | `0x34` | Addr16 | `addr` | Jump if negative condition is true [1] |
| `JV` | `0x35` | Addr16 | `addr` | Jump if overflow condition is true [1] |
| `CALL` | `0x40` | Addr16 | `addr` | Call subroutine at address [1] |
| `RET` | `0x41` | None | none | Return from subroutine [1] |
| `PUSH` | `0x50` | OneReg | `reg` | Push register value onto stack [1] |
| `POP` | `0x51` | OneReg | `reg` | Pop value from stack into register [1] |
| `OUT` | `0x60` | OneReg | `reg` | Output register char value to device or console [1] |
| `OUTN` | `0x61` | OneReg | `reg` | Output register int value to device or console [1] |
| `HLT` | `0xFF` | None | none | Halt execution [1] |

## Encoding details

The assembler emits instruction bytes in a fixed order determined by `emit` [1]. For one-register instructions, the register index byte follows the opcode [1]. For two-register instructions, the destination register index is emitted first and the source register index second [1].

For `LDI`, the encoding is opcode, register index, then immediate byte [1]. For jumps and calls, the encoding is opcode, high byte of address, then low byte of address, so addresses are emitted in big-endian byte order at the instruction level [1].

Examples:

```asm
LDI A, 10
MOV B, A
JMP start
```

These emit, respectively, an opcode plus register/immediate sequence, an opcode plus two register bytes, and an opcode plus a 16-bit address split high byte then low byte [1].

## Operand rules

Each instruction enforces an exact operand count based on its kind [1]. If a no-operand instruction receives any operands, or if one-register and two-register instructions receive the wrong number of operands, the assembler throws a descriptive error such as `takes no operands`, `needs 1 operand`, or `needs 2 operands` [1].

`LDI` requires two operands: a register and an 8-bit immediate [1]. If the immediate value is greater than `0xFF`, the assembler rejects it with `8-bit immediate out of range` [1].

Address-based instructions such as `JMP`, `JZ`, `JNZ`, `JC`, `JN`, `JV`, and `CALL` require exactly one operand and that operand is resolved as a 16-bit value, usually either a numeric address or a label [1].

## Writing programs

A typical assembly program starts with an optional `.ORG`, defines labels where useful, uses instructions line by line, and may include `.DB` data blocks [1]. Because labels are symbolic addresses, branches and calls are usually written using labels instead of raw numeric addresses [1].

A simple example:

```asm
.ORG 0x0000

start:
    LDI A, 5
    LDI B, 1
loop:
    SUB A, B
    JNZ loop
    HLT
```

In this example, `A` is initialized to `5`, `B` to `1`, the loop subtracts `1` from `A`, and `JNZ loop` repeats until the CPU no longer considers the result nonzero according to its flag logic [1].

A second example using data and output:

```asm
.ORG 0x0100

newline: .DB '\n'

start:
    LDI A, 'H'
    OUT A
    LDI A, 'i'
    OUT A
    LDI A, '\n'
    OUT A
    HLT
```

This program demonstrates character literals, `OUT`, and normal sequential execution ending in `HLT` [1].

## Assembler workflow

The assembler follows a classic two-pass structure [1]. In the first pass, it strips comments, trims lines, records labels, interprets `.ORG`, counts bytes for `.DB`, and adds instruction sizes using `instrSize` [1].

In the second pass, it resolves operands and emits the actual bytecode using the `emit` function [1]. This design allows forward references, so a jump to a label declared later in the file still assembles correctly [1].

## Error handling

The assembler reports several concrete errors directly visible in the implementation [1]. These include `could not open input file`, `invalid register`, `empty value`, `invalid char literal`, `invalid binary literal`, `unknown instruction`, and incorrect operand count messages [1].

These errors are useful when writing programs because they fail early and point directly to malformed operands or unsupported syntax [1]. As a result, source programs have a clear, predictable contract with the assembler [1].

## Practical guidance

When writing for this ISA, use labels instead of hard-coded addresses whenever possible, keep immediates within 8-bit range for `LDI`, and prefer register aliases only if they improve readability consistently across the whole program [1]. Because the assembler is case-insensitive, style is mostly a matter of convention, but using uppercase mnemonics and lowercase labels often makes source easier to scan [1].

For larger programs, separate code and data with `.ORG` and clearly named labels [1]. Also keep in mind that the precise runtime meaning of operations such as `CMP`, conditional jumps, stack behavior, and memory access through `LDR` and `STR` depends on the CPU implementation in addition to the assembler interface [1].
