#include "CLSmith/CLProgramGenerator.h"

#include <cassert>
#include <memory>
#include <string>
#include <iostream>

#include "CLSmith/CLExpression.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/Divergence.h"
#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/FunctionInvocationBuiltIn.h"
#include "CLSmith/Globals.h"
#include "CLSmith/StatementBarrier.h"
#include "CLSmith/StatementEMI.h"
#include "CLSmith/Vector.h"
#include "Function.h"
#include "Type.h"

class OutputMgr;

namespace CLSmith {

void CLProgramGenerator::goGenerator() {
  // Initialise probabilies.
  CLExpression::InitProbabilityTable();
  // Create vector types.
  Vector::GenerateVectorTypes();
  // Initialise function tables.
  FunctionInvocationBuiltIn::InitTables();

  // Expects argc, argv and seed. These vars should really be in the output_mgr.
  output_mgr_->OutputHeader(0, NULL, seed_);

  // This creates the random program, the rest handles post-processing and
  // outputting the program.
  GenerateAllTypes();
  GenerateFunctions();

  // If tracking divergence is set, perform the tracking now.
  std::unique_ptr<Divergence> div;
  if (CLOptions::track_divergence()) {
    div.reset(new Divergence());
    div->ProcessEntryFunction(GetFirstFunction());
  }

  // If EMI block generation is set, prune them.
  if (CLOptions::EMI())
    EMIController::GetEMIController()->PruneEMISections();

  // At this point, all the global variables have been created.
  Globals *globals = Globals::GetGlobals();

  // Once the global struct is created, we add the global memory buffers.
  // Adding global memory buffers to do with atomic expressions.
  if (CLOptions::atomics())
    for (MemoryBuffer *mb : *ExpressionAtomic::GetGlobalMems())
      globals->AddGlobalMemoryBuffer(mb);

  // Now add the input data for EMI sections if specified.
  if (CLOptions::EMI())
    for (MemoryBuffer *mb :
        *EMIController::GetEMIController()->GetItemisedEMIInput())
      globals->AddGlobalMemoryBuffer(mb);

  // If barriers have been set, use the divergence information to place them.
  if (CLOptions::barriers()) {
    if (CLOptions::divergence()) GenerateBarriers(div.get(), globals);
    else { /*TODO Non-div barriers*/ }
  }

  // Output the whole program.
  output_mgr_->Output();

  // Release any singleton instances used.
  Globals::ReleaseGlobals();
  EMIController::ReleaseEMIController();
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
