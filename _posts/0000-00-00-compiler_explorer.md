---
title: Compiler Explorer
author: Dr. Matthias Kretz
date: 0000-01-01
layout: post
---


* [Compiler Explorer][1], or similar inspection tools are an invaluable tool for 
  optimization and micro-benchmarking.

* An alternative you might find interesting is the use of local tools to the 
  same end. If you're not familiar with the necessary tools you can use two of 
  the scripts that I created for my own use: [`vir_inspect.sh`][2] (requires 
  zsh `sudo apt install zsh`) and [`vir_dump_asm.sh`][6].

  * ```sh
  vir_inspect.sh /path/to/executable
  ```
  shows a filtered list of functions in the executable. Call
  ```sh
  vir_inspect.sh /path/to/executable <pattern>
  ```
  and it will filter the list of functions using the last argument. If a single 
  function remains it skips the next step.

  * Enter the number of the function you want to inspect.

  * The tool will show a disassembly of the function. If debug information is 
  available (compiled with `-g`), source code annotation will be shown.

  * After the disassembly, [`llvm-mca`][3] will interpret the complete function. 
  This is often not very useful, unless the function was carefully crafted to be 
  interpreted by `llvm-mca`. But feel free to extend the script to insert 
  [`# LLVM-MCA-BEGIN name0` and `# LLVM-MCA-END name0` markers][4] before 
  feeding into `llvm-mca`.

  * `vir_dump_asm.sh <source file>` will compile and dump asm.

* Another alternative for Vim users: I hacked up a Compiler Explorer-like vim 
  plugin for myself. It's available at [vim-compilerexplorer][8]. 

* When looking at x86 asm, I recommend to use Intel syntax instead of AT&T 
  assembler syntax. (Makes it easier when consulting Intel documentation.)

- Quick x86 asm Introduction ([by Matt Godbolt][5]):
  - General purpose registers (integers, pointers):
    `rax`, `rbx`, `rcx`, `rdx`, `rsi`, `rdi`, `rbp`, `rip`, `rsp`, `r8–r15`
  - Floating-point and SIMD registers:
    `xmm0`–`xmm15`
  - ABI-specific registers:
    `rdi`, `rsi`, `rdx`, … as function arguments
    `rax` is the return value
  - `rsp` is the "stack pointer" (`push` and `pop` implicitly modify `rsp`)
  - `rax`, `eax`, `ax`, `ah`, and `al` all alias the same register:
    8 Bytes, 4 Bytes, 2 Bytes, and 1 Bytes (high and low)

- Instructions (`op` here is a placeholder):
  - `op` (often implicit src/dest)
  - `op dest` (often in/out and implicit src)
  - `op dest, src` (often in/out dest)
  - `op dest, src1, src2`

- Load/store/copy instruction (move):
  - `mov eax, edi` (`eax = edi`)
  - `mov eax, DWORD PTR``[rdi+rsi*4]` "load from memory"
    (`eax = *(int*)(rdi + rsi * 4)`)

- Address calculation "load effective address"
  - `lea eax, [rdi+rsi]` (`eax = rdi + rsi`)

- Important patterns:
  - `xor eax, eax`: produces `0`
  - `test edi, edi`: set flags (special register in the CPU)
    `sete al`: Set "a" register to `0` or `1` depending on "equal" (ZF: zero flag)

- Important help:
  - Right click on an instruction on CE: "view assembly documentation"

* Interesting floating-point instructions:
  * All of these instructions may have a `v` prefix (e.g. `vmovss` instead of 
  `movss`), which you can ignore. It's only a different instruction encoding.
  * `movss`: **mov**e **s**calar **s**ingle-precision
    (`op1 = op2`)
  * `addss`: **add** **s**calar **s**ingle-precision
    (`op1 += op2` or `op1 = op2 + op3`)
  * `fmadd132ss`: **f**used **m**ultiply-**add** 132 (argument order:
    `op1 = op1 * op3 + op2`) **s**calar **s**ingle-precision
  * `movd`: **mov**e **d**oubleword (32 bits)
    (`op1 = op2`)
  * `movsd`: **mov**e **s**calar **d**ouble-precision
  * `addsd`: **add** **s**calar **d**ouble-precision

* Later we will also see instructions that use **p**acked instead of **s**calar 
  in their mnemonic. E.g. `addps` instead of `addss`. "packed" means SIMD.


[1]: https://compiler-explorer.com/
[2]: assets/vir_inspect.sh
[3]: https://www.llvm.org/docs/CommandGuide/llvm-mca.html
[4]: https://www.llvm.org/docs/CommandGuide/llvm-mca.html#using-markers-to-analyze-specific-code-blocks
[5]: https://youtu.be/bSkpMdDe4g4?t=8m19s
[6]: assets/vir_dump_asm.sh
[8]: https://github.com/mattkretz/vim-compilerexplorer
