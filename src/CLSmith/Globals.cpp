#include "CLSmith/Globals.h"

#include <memory>
#include <ostream>
#include <string>

#include "ArrayVariable.h"
#include "util.h"
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
  // Two local copies of the global struct. The first will be default
  // initialised, so we can set a pointer to it, allowing our second
  // copy to be brace initialised.
  std::string local_name1 = gensym("c_");
  std::string local_name2 = gensym("c_");
  struct_type_->Output(out);
  out << " " << local_name1 << ";" << std::endl;

  struct_type_ptr_->Output(out);
  out << " ";
  struct_var_->Output(out);
  out << " = &" << local_name1 << ";" << std::endl;

  struct_type_->Output(out);
  out << " " << local_name2 << " = {" << std::endl;
  for (Variable *var : global_vars_) {
    if (var->isArray) {
      ArrayVariable *var_array = dynamic_cast<ArrayVariable *>(var);
      if (!var_array->collective) continue;
      vector<std::string> init_strings;
      init_strings.push_back(var_array->init->to_string());
      for (const Expression *init : var_array->get_more_init_values())
        init_strings.push_back(init->to_string());
      out << var_array->build_init_recursive(0, init_strings);
    } else {
      var->init->Output(out);
    }
    out << ", /*";
    var->Output(out);
    out << "*/" << std::endl;
  }
  out << "};" << std::endl;

  out << local_name1 << " = " << local_name2 << ";" << std::endl;
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
  std::vector<std::string> names;
  for (Variable *var : global_vars_) {
    names.push_back(var->name);
    *const_cast<std::string *>(&var->name) =
        struct_var_->name + "->" + var->name;
  }

  // Possible bug where array variables are being itemised (and so copied) but
  // not put in the global vars list. Have to trawl through all vars.
  // This is a temporary fix, as it can also screw up stuff later.
  for (const std::string& name : names) {
    const Variable *all_var = VariableSelector::find_var_by_name(name);
    while (all_var != NULL) {
      *const_cast<std::string *>(&all_var->name) =
          struct_var_->name + "->" + all_var->name;
      all_var = VariableSelector::find_var_by_name(name);
    }
  }
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
