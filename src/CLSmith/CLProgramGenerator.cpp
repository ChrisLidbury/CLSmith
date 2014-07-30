#include "CLSmith/CLProgramGenerator.h"

#include <cassert>
#include <memory>
#include <string>
#include <iostream>

#include "CLSmith/CLExpression.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/Divergence.h"
#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/Globals.h"
#include "CLSmith/StatementBarrier.h"
#include "CLSmith/Vector.h"
#include "CLSmith/Walker.h"
#include "Function.h"
#include "Type.h"

class OutputMgr;

namespace CLSmith {

void CLProgramGenerator::goGenerator() {
  // Initialise probabilies.
  CLExpression::InitProbabilityTable();
  // Create types.
  Vector::GenerateVectorTypes();

  // Expects argc, argv and seed. These vars should really be in the output_mgr.
  output_mgr_->OutputHeader(0, NULL, seed_);

  // This creates the random program, the rest handles outputting the program.
  GenerateAllTypes();
  GenerateFunctions();

  std::unique_ptr<Divergence> div;
  if (CLOptions::track_divergence()) {
    div.reset(new Divergence());
    div->ProcessEntryFunction(GetFirstFunction());
  }

  Globals *globals = Globals::GetGlobals();
  // Once the global struct is created, we add the global memory buffers.
  // Adding global memory buffers to do with atomic expressions
  for (MemoryBuffer * mb : *ExpressionAtomic::GetGlobalMems())
    globals->AddGlobalMemoryBuffer(mb);

  if (CLOptions::barriers()) {
    if (CLOptions::divergence()) GenerateBarriers(div.get(), globals);
    else { /*TODO Non-div barriers*/ }
  }

  output_mgr_->Output();
  Globals::ReleaseGlobals();
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
