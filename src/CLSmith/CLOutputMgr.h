// Outputs to the designated stream such that the resulting file can be compiled
// as OpenCL C.
// Relies on the caller to disable parts of the program that produce invalid
// OpenCL C, as this class will call the output function of the standard csmith
// output managers.

#ifndef _CLSMITH_CLOUTPUTMGR_H_
#define _CLSMITH_CLOUTPUTMGR_H_

#include <fstream>
#include <string>

//#include "CLSmith/Util.h"
#include "CommonMacros.h"
#include "OutputMgr.h"

namespace CLSmith {

class CLOutputMgr : public OutputMgr {
 public:
  CLOutputMgr();
  explicit CLOutputMgr(const std::string& filename) : out_(filename.c_str()) {}
  explicit CLOutputMgr(const char *filename) : out_(filename) {}

  // Inherited from OutputMgr. Outputs comments, #defines and forward
  // declarations.
  void OutputHeader(int argc, char *argv[], unsigned long seed);
  // Inherited from OutputMgr. Outputs all the definitions.
  void Output();

  // Inherited from OutputMgr. Gets the stream used for printing the output.
  std::ostream &get_main_out();

 private:
  std::ofstream out_;

  DISALLOW_COPY_AND_ASSIGN(CLOutputMgr);
};

}  // namespace CLSmith

#endif  // _CLSMITH_CLOUTPUTMGR_H_
