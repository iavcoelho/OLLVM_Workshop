# Arithmetic Obfuscaton
This is a simple implementation of a LLVM pass that obfuscates arithmetic operations in the LLVM IR code. The pass replaces the operations: +, -, ^, &, | with equivalent but more complex expressions.

The substitutions are as follows:

| Operation | MBA Transformation     |
|-----------|------------------------|
| X ^ Y     | (X \| Y) - (X & Y)     |
| X + Y     | (X & Y) + (X \| Y)     |
| X - Y     | (X ^ -Y) + 2*(X & -Y)  |
| X & Y     | (X + Y) - (X \| Y)     |
| X \| Y    | X + Y + 1 + (~X \| ~Y) |

For example, the expression `a + b` would be transformed to `(a & b) + (a | b)`. And if we consequently apply the transformation to `a & b` and `a | b`, we would get:

`a + b  ->  (a & b) + (a | b) ->  (a & b) + (a + b + 1 + (~a | ~b))`

We can apply the transformations multiple times to increase the complexity of the expressions, being the minimum recommended 2 iterations.

The implementation first iterates over all the instructions in each basic block of the function. If an instruction is a binary operation (like addition, subtraction, etc.), it checks the opcode of the operation. 

```cpp
<SNIP>
namespace {
    struct ArithmeticObf : public PassInfoMixin<ArithmeticObf> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            for (auto& F : M) {
                for (auto& BB : F) {
                    for (auto& I : BB) {
                        if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {                       // Check if the instruction is a binary operation
                            if (binOp->getOpcode() == Instruction::Add) {                       // Check if it's an addition operation
                                    errs() << "Found add instruction: " << *binOp << "\n";
                            } else if (binOp->getOpcode() == Instruction::Sub) {                // Check if it's a subtraction operation
                                    errs() << "Found sub instruction: " << *binOp << "\n";
                            } else if (binOp->getOpcode() == Instruction::Xor) {               // Check if it's a XOR operation
                                    errs() << "Found xor instruction: " << *binOp << "\n";
                            } else if (binOp->getOpcode() == Instruction::And) {               // Check if it's an AND operation
                                    errs() << "Found and instruction: " << *binOp << "\n";
                            } else if (binOp->getOpcode() == Instruction::Or) {                // Check if it's an OR operation
                                    errs() << "Found or instruction: " << *binOp << "\n";
                            }
                        }
                    }
                }
            }
            return PreservedAnalyses::all();
        }
    };
}
```

If the opcode matches one of the operations we want to obfuscate, it creates a new expression using the corresponding MBA transformation and replaces all uses of the original instruction with the new expression.
```cpp
// MBA for X ^ Y = (X | Y) - (X & Y)
Value* mba_xor(Value* X, Value* Y, IRBuilder<>& builder) {
    Value* orInst = builder.CreateOr(X, Y, "or_tmp");
    Value* andInst = builder.CreateAnd(X, Y, "and_tmp");
    Value* subInst = builder.CreateSub(orInst, andInst, "xor_mba");
    return subInst;
}

// MBA for X + Y = (X & Y) + (X | Y)
Value* mba_add(Value* X, Value* Y, IRBuilder<>& builder) {
    Value* andInst = builder.CreateAnd(X, Y, "and_tmp");
    Value* orInst = builder.CreateOr(X, Y, "or_tmp");
    Value* addInst = builder.CreateAdd(andInst, orInst, "add_mba");
    return addInst;
}

// MBA for X - Y = (X ^ -Y) + 2*(X & -Y)
Value* mba_sub(Value* X, Value* Y, IRBuilder<>& builder) {
    Value* negY = builder.CreateNeg(Y, "neg_tmp");
    Value* xorInst = builder.CreateXor(X, negY, "xor_tmp");
    Value* andInst = builder.CreateAnd(X, negY, "and_tmp");
    Value* shlInst = builder.CreateShl(andInst, ConstantInt::get(X->getType(), 1), "shl_tmp");
    Value* addInst = builder.CreateAdd(xorInst, shlInst, "sub_mba");
    return addInst;
}

// MBA for X & Y = (X + Y) - (X | Y)
Value* mba_and(Value* X, Value* Y, IRBuilder<>& builder) {
    Value* addInst = builder.CreateAdd(X, Y, "add_tmp");
    Value* orInst = builder.CreateOr(X, Y, "or_tmp");
    Value* subInst = builder.CreateSub(addInst, orInst, "and_mba");
    return subInst;
}

// MBA for X | Y = X + Y + 1 + (~X | ~Y)
Value* mba_or(Value* X, Value* Y, IRBuilder<>& builder) {
    Value* addInst = builder.CreateAdd(X, Y, "add_tmp");
    Value* notX = builder.CreateNot(X, "notX_tmp");
    Value* notY = builder.CreateNot(Y, "notY_tmp");
    Value* orInst = builder.CreateOr(notX, notY, "or_tmp");
    Value* addOne = builder.CreateAdd(addInst, ConstantInt::get(X->getType(), 1), "addOne_tmp");
    Value* finalAdd = builder.CreateAdd(addOne, orInst, "or_mba");
    return finalAdd;
}
```