#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h" 
#include "llvm/IR/Dominators.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include <assert.h>
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<TargetLibraryInfoWrapperPass>();
    }

    virtual bool runOnFunction(Function &F) {

      // craete llvm function with
      LLVMContext &Ctx = F.getContext();
      std::vector<Type*> paramTypes = {Type::getInt32Ty(Ctx)};
      Type *retType = Type::getVoidTy(Ctx);
      FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
      Module* module = F.getParent();
      Constant *logFunc = module->getOrInsertFunction("logop", logFuncType);

      // perform constant prop and loop analysis
      auto constProp = createConstantPropagationPass();
      bool changed = constProp->runOnFunction(F);
      LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

      // induction var canonicalization  
      PassManagerBuilder builder;
      builder.OptLevel = 0;
      legacy::FunctionPassManager FPM(module);
      FPM.add(createIndVarSimplifyPass());
      FPM.run(F);

      // induction var reduction
      for(auto* L : LI){
        if (PHINode *Node = L->getCanonicalInductionVariable()) {
          errs() << Node->getName() <<"\n";
          // auto workList = L->getBlocksSet();
          // SmallVector<Instruction *, 16> WorkListVec;
          // for (Instruction &I : instructions(*loop)) {
          //   WorkListVec.push_back(&I);
          // }
        }        

        bool mutate = false;
        for (auto &B : F) {
          BasicBlock* b = &B;
          // errs() << "analyze bb: " << B.getName() << "\n";
          // analyze basic blocks with loops
          if (auto bi = dyn_cast<BranchInst>(B.getTerminator())) {
            // Value *loopCond = bi->getCondition();
            for (auto &I : B) {
              if(isa<CallInst>(&I) || isa<InvokeInst>(&I)){
                errs() << cast<CallInst>(&I)->getCalledFunction()->getName() << "\n";
              }
              if (auto *op = dyn_cast<BinaryOperator>(&I)) {

                IRBuilder<> builder(op); // ir builder at op
                Value *lhs = op->getOperand(0);
                Value *rhs = op->getOperand(1);
                Value *mul = builder.CreateMul(lhs, rhs);
  
                mutate = true;
                for (auto &U : op->uses()) {
                  User *user = U.getUser();  
                  // user->setOperand(U.getOperandNo(), mul);
                }
              }
            }
          }
        }
      }
      // dump the mutated llvm ir & execute 
      std::error_code ecode;
      raw_fd_ostream dest("opt.ll", ecode);
      module->print(dest, nullptr);
      return true;
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
