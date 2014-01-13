#include "CLSmith/Globals.h"

#include <memory>
#include <ostream>
#include <string>

#include "CVQualifiers.h"
#include "Type.h"
#include "Function.h"
#include "Variable.h"
#include "VariableSelector.h"

namespace CLSmith {

Globals::Globals(Globals&& other) {
  std::swap(global_vars_, other.global_vars_);
  std::swap(struct_type_, other.struct_type_);
  std::swap(struct_type_, other.struct_type_ptr_);
}

Globals& Globals::operator=(Globals&& other) {
  assert(this != &other);
  std::swap(global_vars_, other.global_vars_);
  std::swap(struct_type_, other.struct_type_);
  std::swap(struct_type_, other.struct_type_ptr_);
  return *this;
}

void Globals::OutputStructDefinition(std::ostream& out) {
  if (!struct_type_) CreateType();
  OutputStructUnion(struct_type_.get(), out);
}

void Globals::OutputStructInit(std::ostream& out) {

}

void Globals::AddGlobalStructToFunction(Function *function, Variable *var) {
  function->param.push_back(var);
}

void Globals::AddGlobalStructToAllFunctions() {
  if (!struct_type_) CreateType();

  // The struct we are adding as a param is a pointer to our global struct that
  // is non-const and non-volatile.
  vector<bool> const_qfer({false, false});
  vector<bool> volatile_qfer({false, false});
  CVQualifiers qfer(const_qfer, volatile_qfer);
  /*std::unique_ptr<Variable> struct_var(*/ Variable *struct_var =
      VariableSelector::GenerateParameterVariable(
      struct_type_ptr_.get(), &qfer);

  // Append the name of our newly created struct to the front of every global
  // variable (eugghhh).
  for (Variable *var : global_vars_)
    *const_cast<std::string *>(&var->name) =
        struct_var->name + "->" + var->name;

  const std::vector<Function *>& functions = get_all_functions();
  for (Function *function : functions)
    AddGlobalStructToFunction(function, struct_var);
}

Globals Globals::CreateGlobals() {
  return Globals(*VariableSelector::GetGlobalVariables());
}

void Globals::CreateType() {
  std::vector<const Type *> types;
  std::vector<CVQualifiers> qfers;
  std::vector<int> bitfield_len;
  for (Variable *var : global_vars_) {
    types.push_back(var->type);
    qfers.push_back(var->qfer);
    bitfield_len.push_back(-1);
  }
  struct_type_.reset(new Type(types, true, false, qfers, bitfield_len));
  struct_type_ptr_.reset(new Type(struct_type_.get()));
}

}  // namespace CLSmith
