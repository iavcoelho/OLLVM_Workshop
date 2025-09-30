# Pass that randomizes register allocation
This pass randomizes the register allocation of instructions in the LLVM IR code. It does so by iterating over all instructions in each basic block of every function and randomly swapping the registers used in binary operations.

Example intel assembly before the pass:
```asm
mov     rcx, 3Eh
mov     rdx, 4Fh
lea     r8, [rbp+var]
```

Example intel assembly after the pass:
```asm
mov     r12, 3Eh
mov     r13, 4Fh
lea     r14, [rbp+var]
```

To achieve this, we first iterate over all the instructions in each basic block of every function using the `BasicBlock` class. We then check if the instruction is a `mov` or `lea` instruction and if so, we randomly select a different register to use.

```cpp
for (auto &I : BB) {
    // Check if the instruction is a mov or lea instruction
    if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
}
```