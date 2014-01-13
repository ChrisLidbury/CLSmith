#include "CLSmith/CLProgramGenerator.h"

#include <cassert>
#include <memory>
#include <string>

//#include "CLSmith/CLFunction.h"
#include "CLSmith/Globals.h"
#include "Function.h"
#include "Type.h"

class OutputMgr;

namespace CLSmith {

void CLProgramGenerator::goGenerator() {
  // Expects argc, argv and seed. These vars should really be in the output_mgr.
  output_mgr_->OutputHeader(0, NULL, 0);

  // This creates the random program, the rest handles outputting the program.
  GenerateAllTypes();
  GenerateFunctions();

  // csmith has now built the program, so we modify as required.
  //ConvertAllFunctions();
  Globals globals = Globals::CreateGlobals();
  globals.AddGlobalStructToAllFunctions();
  //globals.OutputStructDefinition(output_mgr_->get_main_out());

  output_mgr_->Output();
}

OutputMgr *CLProgramGenerator::getOutputMgr() {
  return output_mgr_.get();
}

std::string CLProgramGenerator::get_count_prefix(const std::string& name) {
  assert(false);
  return "";
}

void CLProgramGenerator::initialize() {
}

}  // namespace CLSmith
