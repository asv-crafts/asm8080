# i8080asm

A small cross assembler for the Intel 8080 CPU written in C++. It takes a
plain text assembly source and produces a binary image in the RKS format
used by the Radio-86RK home computer family emulators.

The dialect is largely compatible with the MIKRON assembler shipped with
Radio-86RK, so existing sources written for MIKRON should mostly assemble
without changes.

## RKS file format

The output file layout is straightforward:

```
+0:  start address  (uint16, little endian)
+2:  last address   (uint16, little endian)
+4:  binary payload
+N:  checksum       (uint16, little endian, appended at end of file)
```

## Building

The project has no external dependencies beyond a C++ compiler and GNU
make.

```
make
```

Produces the `i8080asm` binary in the current directory.

To install system-wide (writes to `/usr/local/bin` by default — override
`PREFIX` if needed):

```
sudo make install
```

Clean up build artifacts with `make clean`.

## Usage

```
i8080asm <input source file> <output RKS file> [debug info file]
```

* `input source file` — assembly source to compile.
* `output RKS file` — destination for the RKS binary image.
* `debug info file` — optional. If given, a text file describing labels
  and their comments is written next to the binary. It can be consumed by
  external tools (e.g. a disassembler / debugger) to recover symbolic
  information.

Example:

```
i8080asm examples/example.asm example.rks example.hints
```

## Source syntax

Each line is one of:

* an empty line (ignored)
* a comment line starting with `;`
* a label declaration: `name:` (optionally followed by an instruction)
* an instruction or directive, optionally preceded by a label
* a preprocessor directive starting with `#` in the first column

Labels and mnemonics are case-insensitive. A single address may carry
several labels (each on its own line).

### Numeric literals

* Decimal: `42`
* Hexadecimal: trailing `h`, e.g. `1200h`, `0aa55h`
* Character / string: double quotes, e.g. `"Hello, world!"`

### Expressions

Operands accept arithmetic expressions with two precedence levels:

* High priority: `*`, `/`
* Low priority: `+`, `-`
* Parentheses `(...)` override precedence

Examples:

```
lxi b, 2 + 2 * 2     ; evaluates to 6
lxi d, (2 + 2) * 2   ; evaluates to 8
```

Labels may be used anywhere a number is expected and are resolved to
their address.

### Instructions

All standard Intel 8080 mnemonics are supported, including:

`NOP`, `LXI`, `STAX`, `INX`, `INR`, `DCR`, `MVI`, `RLC`, `DAD`, `LDAX`,
`DCX`, `RRC`, `RAL`, `RAR`, `SHLD`, `DAA`, `LHLD`, `CMA`, `STA`, `STC`,
`LDA`, `CMC`, `MOV`, `HLT`, `ADD`, `ADC`, `SUB`, `SBB`, `ANA`, `XRA`,
`ORA`, `CMP`, `RNZ`, `POP`, `JNZ`, `JMP`, `CNZ`, `PUSH`, `ADI`, `RST`,
`RZ`, `RET`, `JZ`, `CZ`, `CALL`, `ACI`, `RNC`, `JNC`, `OUT`, `CNC`,
`SUI`, `RC`, `JC`, `IN`, `CC`, `SBI`, `RPO`, `JPO`, `XTHL`, `CPO`,
`ANI`, `RPE`, `PCHL`, `JPE`, `XCHG`, `CPE`, `XRI`, `RP`, `JP`, `DI`,
`CP`, `ORI`, `RM`, `SPHL`, `JM`, `EI`, `CM`, `CPI`.

Register operands use the conventional names `A`, `B`, `C`, `D`, `E`,
`H`, `L`, `M`. Register pairs are written as `B`/`BC`, `D`/`DE`,
`H`/`HL`, `SP` (and `PSW` for `PUSH`/`POP`).

### Directives

| Directive | Purpose                                                  |
|-----------|----------------------------------------------------------|
| `ORG`     | Set the origin / current assembly address.               |
| `EQU`     | Bind a label to a constant value (`name equ <expr>`).    |
| `DB`      | Emit one or more bytes, optionally quoted strings.       |
| `DW`      | Emit one or more 16-bit little-endian words.             |
| `DS`      | Reserve N bytes; with two arguments, fill with a value.  |
| `END`     | Terminate assembly. Further lines are ignored.           |

`DB` accepts a comma-separated mixture of byte expressions and quoted
strings on the same line, e.g. `db "Hi", 0, 0aah`.

`DS` accepts either one argument (`ds 5` — five zero bytes) or two
(`ds 5, 77h` — five bytes of `77h`).

### Preprocessor directives

Lines that begin with `#` in the first column are handled by the
preprocessor before reaching the assembler:

| Directive            | Effect                                                     |
|----------------------|------------------------------------------------------------|
| `#include "file"`    | Inline another assembly source at this point.              |
| `#link "file"`       | Append the raw contents of a binary file to the output.    |

Circular includes are detected and reported as an error.

## Example

See `examples/example.asm` for a small program that exercises labels,
expressions, data directives and `#include`. A minimal session:

```
$ make
$ ./i8080asm examples/example.asm out.rks out.hints
```

`out.rks` is the RKS-formatted binary, `out.hints` contains label
addresses and their associated comments.

## Debug information file

When the optional third argument is supplied, the assembler writes a
plain-text "hints" file describing the symbols and comments found in the
source. Each entry has the form:

```
labelName:HHHH;optional comment text
```

where `HHHH` is the four-digit hexadecimal address of the label. Inline
comments not attached to a label are emitted as `:HHHH;comment`. The
same format can be loaded back via `DebugInfo::loadInfo` by downstream
tools.

## Project layout

* `main.cpp` — command-line front end.
* `Preprocessor.{cpp,h}` — handles `#include` / `#link` and drives the
  two compilation passes.
* `Source.{cpp,h}` — line-buffered source file reader.
* `i8080asm.{cpp,h}` — the assembler itself: lexing, expression
  evaluation, label resolution and code emission.
* `Binarizer.{cpp,h}`, `FileBinarizer.{cpp,h}`, `StringBinarizer.{cpp,h}`
  — output sinks. `FileBinarizer` writes the RKS header and trailing
  checksum.
* `DebugInfo.{cpp,h}` — collects labels and comments, reads/writes the
  hints file.
* `common.{cpp,h}` — shared lexical helpers.
* `examples/` — sample sources.

## License

See `LICENSE`.
