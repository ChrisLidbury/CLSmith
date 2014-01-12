// Handles the global variables in the program.
// Actually, there are no global variable in an OpenCL progrma, this handles
// transforming global variables into non-global variables without changing the
// behaviour of the program.
//
// Take the following program:
// 
// int g_1 = 0;
// int g_2 = 1;
// void func_1(void) {
//   g_1 = g_2;
// }
//
// This is not valid in OpenCL, due to g_1 and g_2 being global, we transform it
// into:
//
// struct global {
//   int g_1;
//   int g_2;
// };
// void func_1(struct global *g) {
//   g->g_1 = g->g_2;
// }
//
// The global struct will be initialised by the kernel entry function, and
// passed as a parameter to all the functions in the program.

#ifndef _CLSMITH_GLOBALS_H_
#define _CLSMITH_GLOBALS_H_

#include <algorithm>
#include <ostream>
#include <vector>

#include "CommonMacros.h"

class Variable;

namespace CLSmith {

// Holds pointers to all of the global variables in the program, does not own
// the data that the pointers point to.
class Globals {
 public:
  Globals() {};
  explicit Globals(const std::vector<Variable *>& vars) : global_vars_(vars) {}

  // Move constructors.
  Globals(Globals&& other) { std::swap(global_vars_, other.global_vars_); }
  void operator=(Globals&& other) {
    std::swap(global_vars_, other.global_vars_);
  }

  ~Globals() {}

  // Adds the global variables specified in the parameter.
  void AddGlobalVariable(Variable *var) { global_vars_.push_back(var); }
  void AddGlobalVariables(const std::vector<Variable *> vars) {
    global_vars_.insert(global_vars_.end(), vars.begin(), vars.end());
  }

  // Output the definition of the global struct, including the types of all the
  // variables that have need added.
  void OutputStructDefinition(std::ostream& out);

  // Output an initialisation of the global struct. Assigning the value of each
  // element of the struct.
  void OutputStructInit(std::ostream& out);

  // Must be called after csmith has generated the program. Collects all the
  // global variables in the program.
  static Globals CreateGlobals();

 private:
  std::vector<Variable *> global_vars_;

  DISALLOW_COPY_AND_ASSIGN(Globals);
};

}  // namespace CLSmith

#endif  // _CLSMITH_GLOBALS_H_
