#include "CLSmith/Globals.h"

#include <memory>
#include <ostream>
#include <string>

#include "ArrayVariable.h"
#include "util.h"
#include "CVQualifiers.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "Function.h"
#include "FunctionInvocationUser.h"
#include "Type.h"
#include "Variable.h"
#include "VariableSelector.h"

namespace CLSmith {

void Globals::OutputStructDefinition(std::ostream& out) {
  if (!struct_type_) CreateGlobalStruct();
  struct_type_->Output(out);
  out << " {" << std::endl;
  for (Variable *var : global_vars_) {
    // Need this check, or we will output arrays twice.
    if (var->isArray && dynamic_cast<ArrayVariable *>(var)->collective)
      continue;
    output_tab(out, 1);
    var->OutputDecl(out);
    out << ";" << std::endl;
  }
  out << "};" << std::endl;
}

void Globals::OutputStructInit(std::ostream& out) {
  output_tab(out, 1);
  std::string local_name = gensym("c_");
  struct_type_->Output(out);
  out << " " << local_name << ";" << std::endl;

  output_tab(out, 1);
  struct_type_ptr_->Output(out);
  out << " ";
  struct_var_->Output(out);
  out << " = &" << local_name << ";" << std::endl;

  size_t max_dimen = Variable::GetMaxArrayDimension(global_vars_);
  std::vector<const Variable *>&ctrl_vars = Variable::get_new_ctrl_vars();
  OutputArrayCtrlVars(ctrl_vars, out, max_dimen, 1);
  for (Variable *var : global_vars_) {
    if (var->isArray) {
      ArrayVariable *var_array = dynamic_cast<ArrayVariable *>(var);
      assert(var_array != NULL);
      var_array->output_init(out, var_array->init, ctrl_vars, 1);
    } else {
      output_tab(out, 1);
      var->Output(out);
      out << " = ";
      var->init->Output(out);
      out << ";" << std::endl;
    }
  }
}

void Globals::AddGlobalStructToFunction(Function *function, Variable *var) {
  function->param.push_back(var);
}

void Globals::AddGlobalStructToAllFunctions() {
  if (!struct_type_) CreateGlobalStruct();
  const std::vector<Function *>& functions = get_all_functions();
  for (Function *function : functions)
    AddGlobalStructToFunction(function, struct_var_);

  // Also need to add the parameter to all invocations of the functions.
  for (FunctionInvocationUser *invocation :
      *FunctionInvocationUser::GetAllFunctionInvocations())
    invocation->param_value.push_back(new ExpressionVariable(*struct_var_));
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

  // At some points during generation, a global array variable will be
  // 'itemized'. It will not be put in the list of global variables, so as a
  // fix, we add a method in the VariableSelector that give us the list of all
  // variables.
  // Variable::is_global() is unreliable.
  for (Variable *var : *VariableSelector::GetAllVariables())
    if (var->name.find("g_") == 0)
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
