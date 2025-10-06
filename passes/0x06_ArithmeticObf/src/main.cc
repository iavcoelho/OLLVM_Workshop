#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

#define ITERNUM 2

using namespace llvm;

namespace {
    // MBA for X ^ Y = (X | Y) - (X & Y)
    Value* mba_xor(Value* X, Value* Y, IRBuilder<>& builder) {
        Value* orInst = builder.CreateOr(X, Y, "or_tmp");
        Value* andInst = builder.CreateAnd(X, Y, "and_tmp");
        return builder.CreateSub(orInst, andInst, "xor_mba");
    }

    // MBA for X + Y = (X & Y) + (X | Y)
    Value* mba_add(Value* X, Value* Y, IRBuilder<>& builder) {
        Value* andInst = builder.CreateAnd(X, Y, "and_tmp");
        Value* orInst = builder.CreateOr(X, Y, "or_tmp");
        return builder.CreateAdd(andInst, orInst, "add_mba");
    }

    // MBA for X - Y = (X ^ -Y) + 2*(X & -Y)
    Value* mba_sub(Value* X, Value* Y, IRBuilder<>& builder) {
        Value* negY = builder.CreateNeg(Y, "neg_tmp");
        Value* xorInst = builder.CreateXor(X, negY, "xor_tmp");
        Value* andInst = builder.CreateAnd(X, negY, "and_tmp");
        Value* shlInst = builder.CreateShl(andInst, ConstantInt::get(X->getType(), 1), "shl_tmp");
        return builder.CreateAdd(xorInst, shlInst, "sub_mba");
    }

    // MBA for X & Y = (X + Y) - (X | Y)
    Value* mba_and(Value* X, Value* Y, IRBuilder<>& builder) {
        Value* addInst = builder.CreateAdd(X, Y, "add_tmp");
        Value* orInst = builder.CreateOr(X, Y, "or_tmp");
        return builder.CreateSub(addInst, orInst, "and_mba");
    }

    // MBA for X | Y = X + Y + 1 + (~X | ~Y)
    Value* mba_or(Value* X, Value* Y, IRBuilder<>& builder) {
        Value* addInst = builder.CreateAdd(X, Y, "add_tmp");
        Value* notX = builder.CreateNot(X, "notX_tmp");
        Value* notY = builder.CreateNot(Y, "notY_tmp");
        Value* orInst = builder.CreateOr(notX, notY, "or_tmp");
        Value* addOne = builder.CreateAdd(addInst, ConstantInt::get(X->getType(), 1), "addOne_tmp");
        return builder.CreateAdd(addOne, orInst, "or_mba");
    }

    struct ArithmeticObf : public PassInfoMixin<ArithmeticObf> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            for (unsigned i = 0; i < ITERNUM; ++i) {
                errs() << "--- Iteration " << i + 1 << "/" << ITERNUM << " ---\n";
                for (Function &F : M) {
                    std::vector<Instruction*> toRemove;
                    for (BasicBlock &BB : F) {
                        for (Instruction &I : BB) {
                            if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
                                IRBuilder<> builder(binOp);
                                Value* lhs = binOp->getOperand(0);
                                Value* rhs = binOp->getOperand(1);
                                Value* newInst = nullptr;

                                switch (binOp->getOpcode()) {
                                    case Instruction::Add:
                                        errs() << "Obfuscating add instruction: " << *binOp << "\n";
                                        newInst = mba_add(lhs, rhs, builder);
                                        break;
                                    case Instruction::Sub:
                                        errs() << "Obfuscating sub instruction: " << *binOp << "\n";
                                        newInst = mba_sub(lhs, rhs, builder);
                                        break;
                                    case Instruction::Xor:
                                        errs() << "Obfuscating xor instruction: " << *binOp << "\n";
                                        newInst = mba_xor(lhs, rhs, builder);
                                        break;
                                    case Instruction::And:
                                        errs() << "Obfuscating and instruction: " << *binOp << "\n";
                                        newInst = mba_and(lhs, rhs, builder);
                                        break;
                                    case Instruction::Or:
                                        errs() << "Obfuscating or instruction: " << *binOp << "\n";
                                        newInst = mba_or(lhs, rhs, builder);
                                        break;
                                    default:
                                        continue;                                                       // Skip non-targeted operations 
                                }

                                binOp->replaceAllUsesWith(newInst);                                     // Redirect the old instruction to use the new one
                                toRemove.push_back(binOp);                                              // Mark the old instruction for removal    

                                errs() << "Replaced with: " << *newInst << "\n";
                            }
                        }
                    }
                    
                    // Safely erase the original instructions after iteration
                    for (Instruction* I : toRemove) {
                        I->eraseFromParent();
                    }
                }
            }
            return PreservedAnalyses::all();
        }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "ArithmeticObf",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(ArithmeticObf());
                });
        }
    };
}