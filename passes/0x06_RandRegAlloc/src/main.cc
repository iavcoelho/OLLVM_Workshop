#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#include <random>
#include <vector>

namespace X86 {
enum : unsigned {
  LEA64r = 2478, MOV64ri = 2908, RAX = 100, RBX = 101, RCX = 102, RDX = 103,
  RDI = 104, RSI = 105, RBP = 106, RSP = 107, R8 = 108, R9 = 109, R10 = 110,
  R11 = 111, R12 = 112, R13 = 113, R14 = 114, R15 = 115,
};
} // namespace X86

using namespace llvm;

struct RandomizeRegs : public PassInfoMixin<RandomizeRegs> {
  PreservedAnalyses run(MachineFunction &MF, MachineFunctionAnalysisManager &MAM) {
    errs() << "Running Register Randomizer (NPM, Named) on function: " << MF.getName() << "\n";
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool modified = false;

    const std::vector<unsigned> candidateRegs = {
        X86::RAX, X86::RBX, X86::RCX, X86::RDX, X86::RSI, X86::RDI,
        X86::R8,  X86::R9,  X86::R10, X86::R11, X86::R12, X86::R13,
        X86::R14, X86::R15};

    std::random_device rd;
    std::mt19937 gen(rd());

    for (MachineBasicBlock &MBB : MF) {
      for (MachineInstr &MI : MBB) {
        if (MI.getOpcode() == X86::MOV64ri || MI.getOpcode() == X86::LEA64r) {
          MachineOperand &destOp = MI.getOperand(0);
          if (!destOp.isReg() || !destOp.isDef()) continue;
          unsigned oldReg = destOp.getReg();
          if (!Register::isPhysicalRegister(oldReg)) continue;

          unsigned newReg = oldReg;
          while (newReg == oldReg) {
            std::uniform_int_distribution<> distrib(0, candidateRegs.size() - 1);
            newReg = candidateRegs[distrib(gen)];
          }

          if (MRI.isPhysRegUsed(newReg)) continue;
          
          errs() << "  Changing: " << MI;
          destOp.setReg(newReg);
          errs() << "        To: " << MI;
          modified = true;
        }
      }
    }
    return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
  static bool isRequired() { return true; }
};

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "RandomizeRegs", "v0.1",
    [](PassBuilder &PB) {
      // This is the only registration API that your environment supports.
      PB.registerPipelineParsingCallback(
        [](StringRef Name, MachineFunctionPassManager &MPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "randregalloc") {
            MPM.addPass(RandomizeRegs());
            return true;
          }
          return false;
        }
      );
    }
  };
}