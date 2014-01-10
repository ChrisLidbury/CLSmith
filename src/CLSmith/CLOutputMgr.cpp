#include "CLSmith/CLOutputMgr.h"

#include <fstream>

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
      "#define UINT8_MAX UCHAR_MAX\n";
  OutputMgr::OutputHeader(argc, argv, seed);
}

void CLOutputMgr::Output() {
  std::ostream &out = get_main_out();
  OutputStructUnionDeclarations(out);
  
  // Only print the variable declarations, not the defs. Wrap them in a struct.
  out << "typedef struct globals {" << std::endl;
  OutputGlobalVariablesDecls(out);
  out << "} g;" << std::endl;

  OutputForwardDeclarations(out);
  OutputFunctions(out);
}

std::ostream& CLOutputMgr::get_main_out() {
  return out_;
}

}  // namespace CLSmith
