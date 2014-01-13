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
#include <memory>
#include <ostream>
#include <vector>

#include "CommonMacros.h"

class Function;
class Type;
class Variable;

namespace CLSmith {

// Holds pointers to all of the global variables in the program, does not own
// the data that the pointers point to.
class Globals {
 public:
  Globals() {}
  explicit Globals(const std::vector<Variable *>& vars) : global_vars_(vars) {}

  // Move constructor/assignment.
  Globals(Globals&& other);
  Globals& operator=(Globals&& other);

  ~Globals() {}

  // Adds the global variables specified in the parameter.
  void AddGlobalVariable(Variable *var) { global_vars_.push_back(var); }
  void AddGlobalVariables(const std::vector<Variable *>& vars) {
    global_vars_.insert(global_vars_.end(), vars.begin(), vars.end());
  }

  // Output the definition of the global struct, including the types of all the
  // variables that have need added.
  void OutputStructDefinition(std::ostream& out);

  // Output an initialisation of the global struct. Assigning the value of each
  // element of the struct.
  void OutputStructInit(std::ostream& out);

  // Modifies the function such that it no longer refers to global variables,
  // instead, it will refer to members of a local struct. Just adds the name of
  // the local struct to the parameter list of the function.
  void AddGlobalStructToFunction(Function *function, Variable *var);

  // Modifies all functions that have been generated to refer to the local
  // struct instead of global variables.
  //
  // Unfortunately, there is no easy way to change how the variables are
  // referenced. We have to make the local struct we pass around have the same
  // name, as we have to change the name field of the variable directly to add
  // "local_struct->" to the start of every global variable.
  void AddGlobalStructToAllFunctions();

  // Must be called after csmith has generated the program. Collects all the
  // global variables in the program.
  static Globals CreateGlobals();

 private:
  // Constructs the Type object using all the variables that have currently been
  // added. It is assumed that all the global variables in the program have
  // already been created and added.
  void CreateType();

 private:
  std::vector<Variable *> global_vars_;
  // Type class generated lazily, needs all the global variables to have been
  // added before creation.
  std::unique_ptr<Type> struct_type_;
  std::unique_ptr<Type> struct_type_ptr_;

  DISALLOW_COPY_AND_ASSIGN(Globals);
};

}  // namespace CLSmith

#endif  // _CLSMITH_GLOBALS_H_
