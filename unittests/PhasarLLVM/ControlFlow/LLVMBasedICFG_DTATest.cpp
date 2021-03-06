#include <gtest/gtest.h>
#include <phasar/DB/ProjectIRDB.h>
#include <phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h>
#include <phasar/PhasarLLVM/Pointer/LLVMTypeHierarchy.h>
#include <phasar/Utils/LLVMShorthands.h>

using namespace std;
using namespace psr;

class LLVMBasedICFG_DTATest : public ::testing::Test {
protected:
  const std::string pathToLLFiles =
      PhasarDirectory + "build/test/llvm_test_code/";
};

TEST_F(LLVMBasedICFG_DTATest, VirtualCallSite_5) {
  ProjectIRDB IRDB({pathToLLFiles + "call_graphs/virtual_call_5_cpp.ll"},
                   IRDBOptions::WPA);
  IRDB.preprocessIR();
  LLVMTypeHierarchy TH(IRDB);
  LLVMBasedICFG ICFG(TH, IRDB, CallGraphAnalysisType::DTA, {"main"});
  llvm::Function *F = IRDB.getFunction("main");
  llvm::Function *FuncA = IRDB.getFunction("_ZN1A4funcEv");
  llvm::Function *VFuncA = IRDB.getFunction("_ZN1A5VfuncEv");
  llvm::Function *VFuncB = IRDB.getFunction("_ZN1B5VfuncEv");
  ASSERT_TRUE(F);
  ASSERT_TRUE(FuncA);
  ASSERT_TRUE(VFuncA);
  ASSERT_TRUE(VFuncB);

  const llvm::Instruction *I = getNthInstruction(F, 16);
  if (llvm::isa<llvm::CallInst>(I) || llvm::isa<llvm::InvokeInst>(I)) {
    llvm::ImmutableCallSite CS(I);
    set<const llvm::Function *> Callees = ICFG.getCalleesOfCallAt(I);

    ASSERT_TRUE(ICFG.isVirtualFunctionCall(CS));
    ASSERT_EQ(Callees.size(), 2);
    ASSERT_TRUE(Callees.count(VFuncB));
    ASSERT_TRUE(Callees.count(VFuncA));
    ASSERT_TRUE(ICFG.getCallersOf(VFuncA).count(I));
    ASSERT_TRUE(ICFG.getCallersOf(VFuncB).count(I));
  }
}

TEST_F(LLVMBasedICFG_DTATest, VirtualCallSite_6) {
  ProjectIRDB IRDB({pathToLLFiles + "call_graphs/virtual_call_6_cpp.ll"},
                   IRDBOptions::WPA);
  IRDB.preprocessIR();
  LLVMTypeHierarchy TH(IRDB);
  LLVMBasedICFG ICFG(TH, IRDB, CallGraphAnalysisType::DTA, {"main"});
  llvm::Function *F = IRDB.getFunction("main");
  llvm::Function *VFuncA = IRDB.getFunction("_ZN1A5VfuncEv");
  llvm::Function *VFuncB = IRDB.getFunction("_ZN1B5VfuncEv");
  ASSERT_TRUE(F);
  ASSERT_TRUE(VFuncA);
  ASSERT_TRUE(VFuncB);

  const llvm::Instruction *I = getNthInstruction(F, 6);
  set<const llvm::Instruction *> Callers = ICFG.getCallersOf(VFuncA);
  llvm::ImmutableCallSite CS(I);
  ASSERT_EQ(Callers.size(), 1);
  ASSERT_TRUE(Callers.count(I));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
