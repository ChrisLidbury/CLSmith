// Mostly a duplicate of AbsProgramGenerator, however, there is a tight coupling
// between it and the DFS and default program generators, which also have a
// tight coupling between the output managers. We redefine it here so we can use
// our own output manager.

#ifndef _CLSMITH_CLPROGRAMGENERATOR_H_
#define _CLSMITH_CLPROGRAMGENERATOR_H_

#include "AbsProgramGenerator.h"
#include "CLSmith/CLOutputMgr.h"
//#include "CLSmith/Util.h"
#include "CommonMacros.h"

#include <memory>
#include <string>

namespace CLSmith {

class CLProgramGenerator : public AbsProgramGenerator {
 public:
  CLProgramGenerator() : output_mgr_(new CLOutputMgr()) {};
  // Transfer pointer ownership.
  explicit CLProgramGenerator(OutputMgr *output_mgr)
      : output_mgr_(output_mgr) {};

  // Inherited from AbsProgramGenerator. Creates the random program.
  void goGenerator();
  // Inherited from AbsProgramGenerator. Would ideally return const OutputMgr&,
  // but this is inherited from a pure virtual function, we also wnat to accept
  // other managers as a parameter.
  OutputMgr *getOutputMgr();
  // Inherited from AbsProgramGenerator. ?
  std::string get_count_prefix(const std::string& name);

  // Initialise the generator. This one doesn't generally do much, as we assume
  // that the initialise method of the csmith program generators has been
  // called.
  void initialize();

 private:
  std::unique_ptr<OutputMgr> output_mgr_;

  DISALLOW_COPY_AND_ASSIGN(CLProgramGenerator);
};

}  // namespace CLSmith

#endif  // _CLSMITH_CLPROGRAMGENERATOR_H_
