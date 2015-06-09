#include "CLSmith/CLProgramGenerator.h"

#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <iostream>

#include "CLSmith/CLExpression.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/CLVariable.h"
#include "CLSmith/Divergence.h"
#include "CLSmith/ExpressionAtomic.h"
#include "ExpressionID.h"
#include "CLSmith/FunctionInvocationBuiltIn.h"
#include "CLSmith/StatementAtomicResult.h"
#include "CLSmith/Globals.h"
#include "CLSmith/StatementAtomicReduction.h"
#include "CLSmith/StatementBarrier.h"
#include "CLSmith/StatementComm.h"
#include "CLSmith/StatementEMI.h"
#include "CLSmith/StatementMessage.h"
#include "CLSmith/Vector.h"
#include "Function.h"
#include "Type.h"

class OutputMgr;

namespace CLSmith {
namespace {
static const unsigned int max_threads = 10000;
static const unsigned int max_thr_per_dim = 100;
static const unsigned int max_threads_per_group = 256;
static const unsigned int no_dims = 3;
static unsigned int noThreads = 1;
static unsigned int noGroups = 1;
static std::vector<unsigned int> *globalDim;
static std::vector<unsigned int> *localDim;
}  // namespace

void CLProgramGenerator::goGenerator() {
  // Initialise probabilies.
  CLExpression::InitProbabilityTable();
  CLStatement::InitProbabilityTable();
  // Create vector types.
  Vector::GenerateVectorTypes();
  // Initialise function tables.
  FunctionInvocationBuiltIn::InitTables();
  // Initialise Variable objects used for thread identifiers.
  ExpressionID::Initialise();
  // Initialize runtime parameters;
  InitRuntimeParameters();
  // Initalize atomic parameters
  if (CLOptions::atomics())
    ExpressionAtomic::InitAtomics();
  // Initialise buffers used for inter-thread communication.
  StatementComm::InitBuffers();
  // Initialise Message Passing data.
  MessagePassing::Initialise();

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
  if (CLOptions::emi())
    EMIController::GetEMIController()->PruneEMISections();

  // If atomic blocks are generated, add operations for the special values
  if (CLOptions::atomics())
    StatementAtomicResult::GenSpecialVals();

  // If atomic reductions are generated, add the hash buffer
  if (CLOptions::atomic_reductions())
    StatementAtomicReduction::RecordBuffer();

  // If Message Passing is enabled, create orderings and message updates.
  if (CLOptions::message_passing())
    MessagePassing::CreateMessageOrderings();

  // At this point, all the global variables that have been created throughout
  // program generation should have been created. Any global variables added to
  // VariableSelector's global list after this point must be added to globals
  // manually, or the name will not be prepended with the global struct.
  Globals *globals = Globals::GetGlobals();

  // Once the global struct is created, we add the global memory buffers.
  // Adding global memory buffers to do with atomic expressions.
  if (CLOptions::atomics()) {
    ExpressionAtomic::AddVarsToGlobals(globals);
  }

  // Add the reduction variables for atomic reductions to the global struct
  if (CLOptions::atomic_reductions())
    StatementAtomicReduction::AddVarsToGlobals(globals);

  // Now add the input data for EMI sections if specified.
  if (CLOptions::emi())
    for (MemoryBuffer *mb :
        *EMIController::GetEMIController()->GetItemisedEMIInput())
      globals->AddGlobalMemoryBuffer(mb);

  // Add the 9 variables used for the identifiers.
  if (CLOptions::fake_divergence())
    ExpressionID::AddVarsToGlobals(globals);

  // Add buffers used for inter-thread comm.
  if (CLOptions::inter_thread_comm())
    StatementComm::AddVarsToGlobals(globals);

  // Add messages for message passing.
  if (CLOptions::message_passing())
    MessagePassing::AddMessageVarsToGlobals(globals);

  // If barriers have been set, use the divergence information to place them.
  if (CLOptions::barriers()) {
    if (CLOptions::divergence()) GenerateBarriers(div.get(), globals);
    else { /*TODO Non-div barriers*/ }
  }

  if (CLOptions::small())
    CLSmith::CLVariable::ParseUnusedVars();

  // Output the whole program.
  output_mgr_->Output();

  // Release any singleton instances used.
  Globals::ReleaseGlobals();
  EMIController::ReleaseEMIController();
}

void CLProgramGenerator::InitRuntimeParameters() {
  globalDim = new std::vector<unsigned int>(no_dims, 1);
  localDim = new std::vector<unsigned int>(no_dims, 1);
  std::vector<unsigned int>& globalDim = *CLSmith::globalDim;
  std::vector<unsigned int>& localDim = *CLSmith::localDim;
  std::vector<unsigned int> chosen_div;
  std::vector<unsigned int> divisors;
  for (unsigned int i = 0; i < no_dims; i++) {
    globalDim[i] = rnd_upto(std::min(max_thr_per_dim, max_threads / noThreads)) + 1;
    noThreads *= globalDim[i];
  }
  unsigned int curr_thr_p_grp;
  do {
    curr_thr_p_grp = 1;
    for (unsigned int i = 0; i < no_dims; i++) {
      get_divisors(globalDim[i], &divisors);
      localDim[i] = divisors[rnd_upto(divisors.size())];
      curr_thr_p_grp *= localDim[i];
    }
  } while (curr_thr_p_grp > max_threads_per_group);
  for (unsigned int i = 0; i < no_dims; i++)
    noGroups *= globalDim[i] / localDim[i];
}

OutputMgr *CLProgramGenerator::getOutputMgr() {
  return output_mgr_.get();
}

std::string CLProgramGenerator::get_count_prefix(const std::string& name) {
  assert(false);
  return "";
}

const unsigned int CLProgramGenerator::get_threads() {
  return noThreads;
}

const unsigned int CLProgramGenerator::get_groups() {
  return noGroups;
}

const unsigned int CLProgramGenerator::get_total_threads() {
  return noThreads * noGroups;
}

const unsigned int CLProgramGenerator::get_threads_per_group() {
  return noThreads / noGroups;
}

const std::vector<unsigned int>& CLProgramGenerator::get_global_dims() {
  return *globalDim;
}

const std::vector<unsigned int>& CLProgramGenerator::get_local_dims() {
  return *localDim;
}

const unsigned int CLProgramGenerator::get_atomic_blocks_no() {
  return CLSmith::ExpressionAtomic::get_atomic_blocks_no();
}

void CLProgramGenerator::initialize() {
}

void CLProgramGenerator::get_divisors(
    unsigned int val, std::vector<unsigned int> *divisors) {
  divisors->clear();
  divisors->push_back(1); divisors->push_back(val);
  unsigned int i;
  for (i = 2; i < sqrt(val); i++) {
    if (!(val % i)) {
      divisors->push_back(i);
      divisors->push_back(val / i);
    }
  }
  if (i * i == val)
    divisors->push_back(i);
}

}  // namespace CLSmith
