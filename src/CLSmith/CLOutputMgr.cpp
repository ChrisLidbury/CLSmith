#include "CLSmith/CLOutputMgr.h"

#include <fstream>

#include "CLSmith/Globals.h"
#include "Function.h"
#include "OutputMgr.h"
#include "Type.h"
#include "VariableSelector.h"

namespace CLSmith {

const std::string kFileName = "CLProg.c";

CLOutputMgr::CLOutputMgr() : out_(kFileName.c_str()) {
}

void CLOutputMgr::OutputHeader(int argc, char *argv[], unsigned long seed) {
  // Redefine platform independent scalar C types to platform independent scalar
  // OpenCL types.
  std::ostream &out = get_main_out();
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
      << std::endl;

  out << std::endl;
  out << "// Seed: " << seed << std::endl;
  out << std::endl;
  out << "#include \"CLSmith.h\"" << std::endl;
  out << std::endl;
}

void CLOutputMgr::Output() {
  std::ostream &out = get_main_out();
  OutputStructUnionDeclarations(out);

  Globals globals = Globals::CreateGlobals();
  globals.OutputStructDefinition(out);
  globals.ModifyGlobalVariableReferences();
  globals.AddGlobalStructToAllFunctions();

  OutputForwardDeclarations(out);
  OutputFunctions(out);
  OutputEntryFunction(globals);
}

std::ostream& CLOutputMgr::get_main_out() {
  return out_;
}

void CLOutputMgr::OutputEntryFunction(Globals& globals) {
  // Would ideally use the ExtensionMgr, but there is no way to set it to our
  // own custom made one (without modifying the code).
  std::ostream& out = get_main_out();
  out << "__kernel void entry(__global ulong *result) {" << std::endl;
  globals.OutputStructInit(out);

  output_tab(out, 1);
  out << "func_1(";
  globals.GetGlobalStructVar().Output(out);
  out << ");" << std::endl;

  // Handle hashing and outputting.
  output_tab(out, 1);
  out << "uint64_t crc64_context = 0xFFFFFFFFFFFFFFFFUL;" << std::endl;
  output_tab(out, 1);
  out << "int print_hash_value = 0;" << std::endl;
  HashGlobalVariables(out);
  output_tab(out, 1);
  out << "result[get_global_id(0)] = crc64_context ^ 0xFFFFFFFFFFFFFFFFUL;"
      << std::endl;
  out << "}" << std::endl;
}

}  // namespace CLSmith
