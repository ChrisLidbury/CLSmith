#include "CLSmith/CLOutputMgr.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include "CLSmith/CLOptions.h"
#include "CLSmith/CLProgramGenerator.h"
#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/ExpressionID.h"
#include "CLSmith/Globals.h"
#include "CLSmith/StatementBarrier.h"
#include "CLSmith/StatementComm.h"
#include "Function.h"
#include "OutputMgr.h"
#include "Type.h"
#include "VariableSelector.h"

namespace CLSmith {

CLOutputMgr::CLOutputMgr() : out_(CLOptions::output()) {
}

void CLOutputMgr::OutputRuntimeInfo(std::vector<unsigned int> global_dims, std::vector<unsigned int> local_dims) {
  std::ostream &out = get_main_out();
  out << "//";
  if (CLOptions::atomics())
    out << " --atomics " << CLProgramGenerator::get_atomic_blocks_no();
  if (CLOptions::atomic_reductions())
    out << " ---atomic_reductions";
  if (CLOptions::fake_divergence())
    out << " ---fake_divergence";
  if (CLOptions::inter_thread_comm())
    out << " ---inter_thread_comm";
  if (CLOptions::emi())
    out << " ---emi";
  out << " -g ";
  for (std::vector<unsigned int>::iterator it = global_dims.begin(); it < global_dims.end(); it++) {
    out << *it;
    if (it + 1 != global_dims.end())
      out << ",";
  }
  out << " -l ";
  for (std::vector<unsigned int>::iterator it = local_dims.begin(); it < local_dims.end(); it++) {
    out << *it;
    if (it + 1 != local_dims.end())
      out << ",";
  }
  out << std::endl;
}

void CLOutputMgr::OutputHeader(int argc, char *argv[], unsigned long seed) {
  // Redefine platform independent scalar C types to platform independent scalar
  // OpenCL types.
  std::ostream &out = get_main_out();
  OutputRuntimeInfo(CLProgramGenerator::get_global_dims(), CLProgramGenerator::get_local_dims());
  out <<
      "#define int64_t long\n"
      "#define uint64_t ulong\n"
      "#define int_least64_t long\n"
      "#define uint_least64_t ulong\n"
      "#define int_fast64_t long\n"
      "#define uint_fast64_t ulong\n"
      "#define intmax_t long\n"
      "#define uintmax_t ulong\n"
      "#define int32_t int\n"
      "#define uint32_t uint\n"
      "#define int16_t short\n"
      "#define uint16_t ushort\n"
      "#define int8_t char\n"
      "#define uint8_t uchar\n"
      "\n"
      "#define INT64_MIN LONG_MIN\n"
      "#define INT64_MAX LONG_MAX\n"
      "#define INT32_MIN INT_MIN\n"
      "#define INT32_MAX INT_MAX\n"
      "#define INT16_MIN SHRT_MIN\n"
      "#define INT16_MAX SHRT_MAX\n"
      "#define INT8_MIN CHAR_MIN\n"
      "#define INT8_MAX CHAR_MAX\n"
      "#define UINT64_MIN ULONG_MIN\n"
      "#define UINT64_MAX ULONG_MAX\n"
      "#define UINT32_MIN UINT_MIN\n"
      "#define UINT32_MAX UINT_MAX\n"
      "#define UINT16_MIN USHRT_MIN\n"
      "#define UINT16_MAX USHRT_MAX\n"
      "#define UINT8_MIN UCHAR_MIN\n"
      "#define UINT8_MAX UCHAR_MAX\n"
      "\n"
      "#define transparent_crc(X, Y, Z) "
      "transparent_crc_(&crc64_context, X, Y, Z)\n"
      "\n"
      "#define VECTOR(X , Y) VECTOR_(X, Y)\n"
      "#define VECTOR_(X, Y) X##Y\n"
      << std::endl;

  // Macro for expanding GROUP_DIVERGE
  ExpressionIDGroupDiverge::OutputGroupDivergenceMacro(out);
  out << std::endl;
  // Macro for expanding FAKE_DIVERGE.
  ExpressionIDFakeDiverge::OutputFakeDivergenceMacro(out);
  out << std::endl;

  // Permuation buffers for inter-thread comm.
  if (CLOptions::inter_thread_comm())
    StatementComm::OutputPermutations(out);

  out << std::endl;
  out << "// Seed: " << seed << std::endl;
  out << std::endl;
  out << "#include \"CLSmith.h\"" << std::endl;
  if (CLOptions::embedded()) {
    out << "#include \"CLSmith_embedded.h\"" << std::endl << std::endl;
  }
  out << std::endl;
}

void CLOutputMgr::Output() {
  std::ostream &out = get_main_out();
  OutputStructUnionDeclarations(out);

  Globals *globals = Globals::GetGlobals();
  globals->OutputStructDefinition(out);
  globals->ModifyGlobalVariableReferences();
  globals->AddGlobalStructToAllFunctions();

  OutputForwardDeclarations(out);
  OutputFunctions(out);
  OutputEntryFunction(*globals);
}

std::ostream& CLOutputMgr::get_main_out() {
  return out_;
}

void CLOutputMgr::OutputEntryFunction(Globals& globals) {
  // Would ideally use the ExtensionMgr, but there is no way to set it to our
  // own custom made one (without modifying the code).
  std::ostream& out = get_main_out();
  out << "__kernel void entry(__global ulong *result";
  if (CLOptions::atomics()) {
    out << ", __global volatile uint *g_atomic_input";
    out << ", __global volatile uint *g_special_values";
  }
  if (CLOptions::atomic_reductions()) {
    out << ", __global volatile int *g_atomic_reduction";
  }
  if (CLOptions::emi())
    out << ", __global int *emi_input";
  if (CLOptions::fake_divergence())
    out << ", __global int *sequence_input";
  if (CLOptions::inter_thread_comm())
    out << ", __global long *g_comm_values";
  out << ") {" << std::endl;
  globals.OutputArrayControlVars(out);
  globals.OutputBufferInits(out);
  globals.OutputStructInit(out);

  // Block all threads before entering to ensure the local buffers have been
  // initialised.
  output_tab(out, 1);
  StatementBarrier::OutputBarrier(out);
  out << std::endl;

  output_tab(out, 1);
  out << "func_1(";
  globals.GetGlobalStructVar().Output(out);
  out << ");" << std::endl;

  // Block all threads after, to prevent hashing stale values.
  output_tab(out, 1);
  StatementBarrier::OutputBarrier(out);
  out << std::endl;

  // Handle hashing and outputting.
  output_tab(out, 1);
  out << "uint64_t crc64_context = 0xFFFFFFFFFFFFFFFFUL;" << std::endl;
  output_tab(out, 1);
  out << "int print_hash_value = 0;" << std::endl;
  HashGlobalVariables(out);
  if (CLOptions::atomics())
    ExpressionAtomic::OutputHashing(out);
  if (CLOptions::inter_thread_comm()) StatementComm::HashCommValues(out);
  output_tab(out, 1);
  out << "result[get_linear_global_id()] = crc64_context ^ 0xFFFFFFFFFFFFFFFFUL;"
      << std::endl;
  out << "}" << std::endl;
}

}  // namespace CLSmith
