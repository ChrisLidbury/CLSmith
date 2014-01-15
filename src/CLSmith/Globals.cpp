#include "CLSmith/Globals.h"

#include <memory>
#include <ostream>
#include <string>

#include "ArrayVariable.h"
#include "CVQualifiers.h"
#include "Expression.h"
#include "Function.h"
#include "Type.h"
#include "Variable.h"
#include "VariableSelector.h"

namespace CLSmith {

Globals::Globals(Globals&& other) {
  std::swap(global_vars_, other.global_vars_);
  std::swap(struct_type_, other.struct_type_);
  std::swap(struct_type_ptr_, other.struct_type_ptr_);
  std::swap(struct_var_, other.struct_var_);
}

Globals& Globals::operator=(Globals&& other) {
  assert(this != &other);
  std::swap(global_vars_, other.global_vars_);
  std::swap(struct_type_, other.struct_type_);
  std::swap(struct_type_ptr_, other.struct_type_ptr_);
  std::swap(struct_var_, other.struct_var_);
  return *this;
}

void Globals::OutputStructDefinition(std::ostream& out) {
  if (!struct_type_) CreateGlobalStruct();
  struct_type_->Output(out);
  out << " {" << std::endl;
  for (Variable *var : global_vars_) {
    // Need this check, or we will output arrays twice.
    if (var->isArray && !dynamic_cast<ArrayVariable *>(var)->collective)
      continue;
    var->OutputDecl(out);
    out << ";" << std::endl;
  }
  out << "};" << std::endl;
}

void Globals::OutputStructInit(std::ostream& out) {
  struct_type_->Output(out);
  out << " ";
  struct_var_->Output(out);
  out << " = {" << std::endl;
  for (Variable *var : global_vars_) {
    if (var->isArray) {
      ArrayVariable *var_array = dynamic_cast<ArrayVariable *>(var);
      if (!var_array->collective) continue;
      vector<std::string> init_strings;
      init_strings.push_back(var_array->init->to_string());
      for (const Expression *init : var_array->get_more_init_values()) init_strings.push_back(init->to_string());
      out << var_array->build_initializer_str(init_strings);
    } else {
      var->init->Output(out);
    }
    out << ", /*";
    var->Output(out);
    out << "*/" << std::endl;
  }
  out << "};" << std::endl;
}

void Globals::AddGlobalStructToFunction(Function *function, Variable *var) {
  function->param.push_back(var);
}

void Globals::AddGlobalStructToAllFunctions() {
  if (!struct_type_) CreateGlobalStruct();
  const std::vector<Function *>& functions = get_all_functions();
  for (Function *function : functions)
    AddGlobalStructToFunction(function, struct_var_);
}

void Globals::ModifyGlobalVariableReferences() {
  if (!struct_type_) CreateGlobalStruct();
  // Append the name of our newly created struct to the front of every global
  // variable (eugghhh).
  for (Variable *var : global_vars_)
    *const_cast<std::string *>(&var->name) =
        struct_var_->name + "->" + var->name;
}

const Type& Globals::GetGlobalStructPtrType() {
  return *struct_type_ptr_.get();
}

const Variable& Globals::GetGlobalStructVar() {
  return *struct_var_;
}

Globals Globals::CreateGlobals() {
  return Globals(*VariableSelector::GetGlobalVariables());
}

void Globals::CreateGlobalStruct() {
  std::vector<const Type *> types;
  std::vector<CVQualifiers> qfers;
  std::vector<int> bitfield_len;
  for (Variable *var : global_vars_) {
    types.push_back(var->type);
    qfers.push_back(var->qfer);
    bitfield_len.push_back(-1);
  }
  struct_type_.reset(new Type(types, true, false, qfers, bitfield_len));
  assert(struct_type_);
  struct_type_ptr_.reset(new Type(struct_type_.get()));
  assert(struct_type_ptr_);

  // Now create the Variable object. It will be a parameter variable that points
  // to our global struct that is non-const and non-volatile.
  vector<bool> const_qfer({false, false});
  vector<bool> volatile_qfer({false, false});
  CVQualifiers qfer(const_qfer, volatile_qfer);
  struct_var_ = VariableSelector::GenerateParameterVariable(
      struct_type_ptr_.get(), &qfer);
  assert(struct_var_);
}

}  // namespace CLSmith
