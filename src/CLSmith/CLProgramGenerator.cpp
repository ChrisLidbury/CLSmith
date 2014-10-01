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
#include "CLSmith/StatementBarrier.h"
#include "CLSmith/StatementComm.h"
#include "CLSmith/StatementEMI.h"
#include "CLSmith/Vector.h"
#include "Function.h"
#include "Type.h"

class OutputMgr;

namespace CLSmith {

static const unsigned int min_no_threads = 100;
static const unsigned int max_no_threads = 10000;
static const unsigned int max_threads_per_group = 256;
static const unsigned int no_dims = 3;
static unsigned int noThreads;
static unsigned int noGroups = 1;
static std::vector<unsigned int> globalDim (no_dims, 1);
static std::vector<unsigned int> localDim (no_dims, 1);

// used for dimension calculation
unsigned int const primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 
  43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 };

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
  
  // If atomic blocks are generated, add operations for the special values
  if (CLOptions::atomics())
    StatementAtomicResult::GenSpecialVals();

  // At this point, all the global variables have been created.
  Globals *globals = Globals::GetGlobals();

  // Once the global struct is created, we add the global memory buffers.
  // Adding global memory buffers to do with atomic expressions.
  if (CLOptions::atomics()) {
    for (MemoryBuffer *mb : *ExpressionAtomic::GetLocalMems()) {
      globals->AddLocalMemoryBuffer(mb);
    }
    for (MemoryBuffer *mb : *ExpressionAtomic::GetLocalSVMems()) {
      globals->AddLocalMemoryBuffer(mb);
    }
    for (MemoryBuffer *mb : *ExpressionAtomic::GetGlobalMems()) {
      globals->AddGlobalMemoryBuffer(mb);
    }
    for (MemoryBuffer *mb : *ExpressionAtomic::GetSVMems()) {
      globals->AddGlobalMemoryBuffer(mb);
    }
  }

  // Now add the input data for EMI sections if specified.
  if (CLOptions::EMI())
    for (MemoryBuffer *mb :
        *EMIController::GetEMIController()->GetItemisedEMIInput())
      globals->AddGlobalMemoryBuffer(mb);

  // Add the 9 variables used for the identifiers.
  if (CLOptions::fake_divergence())
    ExpressionID::AddVarsToGlobals(globals);

  // Add buffers used for inter-thread comm.
  if (CLOptions::inter_thread_comm())
    StatementComm::AddVarsToGlobals(globals);

  // If barriers have been set, use the divergence information to place them.
  if (CLOptions::barriers()) {
    if (CLOptions::divergence()) GenerateBarriers(div.get(), globals);
    else { /*TODO Non-div barriers*/ }
  }
  
  if (CLOptions::small()) {
    CLSmith::CLVariable::ParseUnusedVars();
  }
    

  // Output the whole program.
  output_mgr_->Output();

  // Release any singleton instances used.
  Globals::ReleaseGlobals();
  EMIController::ReleaseEMIController();
}

void CLProgramGenerator::InitRuntimeParameters() {
  noThreads = rnd_upto(max_no_threads - min_no_threads) + min_no_threads;
//   noGroups = rnd_upto(10000);
  std::vector<unsigned int> chosen_div;
  std::vector<unsigned int> divisors;
  int choose, currNo = noThreads, div;
  while (chosen_div.size() < no_dims) {
    divisors = *get_divisors(currNo);
    div = divisors[rnd_upto(divisors.size())];
    chosen_div.push_back(div);
    currNo /= div;
    std::cout << "Selected " << div << std::endl;
  }
  assert(chosen_div.size() == no_dims);
  for (unsigned int i = 0; i < no_dims; i++) {
    choose = rnd_upto(chosen_div.size());
    globalDim[i] = chosen_div[choose];
    chosen_div.erase(std::find(chosen_div.begin(), chosen_div.end(), chosen_div[choose]));
  }
  unsigned int curr_thr_p_grp;
  do {
    curr_thr_p_grp = 1;
    for (unsigned int i = 0; i < no_dims; i++) {
      divisors = *get_divisors(globalDim[i]);
      localDim[i] = divisors[rnd_upto(divisors.size())];
      curr_thr_p_grp *= localDim[i];
    }
  } while (curr_thr_p_grp > max_threads_per_group);
  for (unsigned int i = 0; i < no_dims; i++)
    noGroups *= globalDim[i] / localDim[i];
  
  std::cout << "Threads = " << noThreads << std::endl;
  std::cout << "Groups = " << noGroups << std::endl;
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
  return noGroups / noThreads;
}

const std::vector<unsigned int>& CLProgramGenerator::get_global_dims() {
  return globalDim;
}

const std::vector<unsigned int>& CLProgramGenerator::get_local_dims() {
  return localDim;
}

const unsigned int CLProgramGenerator::get_atomic_blocks_no() {
  return CLSmith::ExpressionAtomic::get_atomic_blocks_no();
}

void CLProgramGenerator::initialize() {
}

std::vector<unsigned int>* CLProgramGenerator::get_divisors(unsigned int val) {
  std::vector<unsigned int>* divisors = new std::vector<unsigned int>();
  divisors->push_back(1); divisors->push_back(val);
  unsigned int max_check = sqrt(val);
  for (unsigned int i = 0; i < sizeof(primes) / sizeof(*primes); i++) {
    if (primes[i] > max_check)
      break;
    if (!(noThreads % primes[i])) {
      divisors->push_back(primes[i]);
      divisors->push_back(val / primes[i]);
    }
  }
  return divisors;
}

}  // namespace CLSmith
