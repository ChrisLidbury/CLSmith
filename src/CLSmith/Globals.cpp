#include "CLSmith/Globals.h"

#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "ArrayVariable.h"
#include "util.h"
#include "CLSmith/MemoryBuffer.h"
#include "CVQualifiers.h"
#include "Expression.h"
#include "ExpressionVariable.h"
#include "Function.h"
#include "FunctionInvocationUser.h"
#include "Type.h"
#include "Variable.h"
#include "VariableSelector.h"

namespace CLSmith {
namespace {
Globals *globals_inst = NULL;  // Singleton instance.
}  // namespace

void Globals::AddLocalMemoryBuffer(MemoryBuffer *buffer) {
  assert(buffer->GetMemorySpace() == MemoryBuffer::kLocal);
  local_buffers_.push_back(buffer);
  if (buffer->collective) return;
  buffer->OutputDef(buff_init_, 1);
  output_tab(struct_buff_init_, 2);
  buffer->Output(struct_buff_init_);
  struct_buff_init_ << ", // ";
  buffer->Output(struct_buff_init_);
  struct_buff_init_ << std::endl;
}

void Globals::AddGlobalMemoryBuffer(MemoryBuffer *buffer) {
  assert(buffer->GetMemorySpace() == MemoryBuffer::kGlobal);
  global_buffers_.push_back(buffer);
  if (buffer->collective) return;
  output_tab(struct_buff_init_, 2);
  buffer->Output(struct_buff_init_);
  struct_buff_init_ << ", // ";
  buffer->Output(struct_buff_init_);
  struct_buff_init_ << std::endl;
}

Globals *Globals::GetGlobals() {
  if (globals_inst == NULL) globals_inst = CreateGlobals();
  return globals_inst;
}

void Globals::ReleaseGlobals() {
  delete globals_inst;
  globals_inst = NULL;
}

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
  for (MemoryBuffer *buffer : local_buffers_) {
    if (buffer->collective) continue;
    output_tab(out, 1);
    buffer->OutputAliasDecl(out);
    out << ";" << std::endl;
  }
  for (MemoryBuffer *buffer : global_buffers_) {
    if (buffer->collective) continue;
    output_tab(out, 1);
    buffer->OutputAliasDecl(out);
    out << ";" << std::endl;
  }
  out << "};" << std::endl;
}

void Globals::OutputStructInit(std::ostream& out) {
  output_tab(out, 1);
  std::string local_name1 = gensym("c_");
  std::string local_name2 = gensym("c_");
  struct_type_->Output(out);
  out << " " << local_name1 << ";" << std::endl;
  output_tab(out, 1);
  struct_type_ptr_->Output(out);
  out << " ";
  struct_var_->Output(out);
  out << " = &" << local_name1 << ";" << std::endl;

  output_tab(out, 1);
  struct_type_->Output(out);
  out << " " << local_name2 << " = {" << std::endl;

  for (Variable *var : global_vars_) {
    if (var->isArray) {
      ArrayVariable *var_array = dynamic_cast<ArrayVariable *>(var);
      if (var_array->collective) continue;
      output_tab(out, 2);
      vector<std::string> init_strings;
      init_strings.push_back(var_array->init->to_string());
      for (const Expression *init : var_array->get_more_init_values())
        init_strings.push_back(init->to_string());
      out << var_array->build_initializer_str(init_strings);
    } else {
      output_tab(out, 2);
      var->init->Output(out);
    }
    out << ", // ";
    var->Output(out);
    out << std::endl;
  }
  out << struct_buff_init_.str();
  struct_buff_init_.clear();

  output_tab(out, 1);
  out << "};" << std::endl;
  output_tab(out, 1);
  out << local_name1 << " = " << local_name2 << ";" << std::endl;
}

void Globals::OutputBufferInits(std::ostream& out) {
  out << buff_init_.str();
  buff_init_.clear();
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
    if (var->is_aggregate()) ModifyGlobalAggregateVariableReferences(var);
  }

  // At some points during generation, a global array variable will be
  // 'itemized'. It will not be put in the list of global variables, so as a
  // fix, we add a method in the VariableSelector that give us the list of all
  // variables.
  // Variable::is_global() is unreliable.
  for (Variable *var : *VariableSelector::GetAllVariables())
    if (var->name.find("g_") == 0 && std::find(global_buffers_.begin(), global_buffers_.end(), var) == global_buffers_.end()) { //?
      *const_cast<std::string *>(&var->name) =
          struct_var_->name + "->" + var->name;
      if (var->is_aggregate()) ModifyGlobalAggregateVariableReferences(var);
    }

  // Now add to the local buffers.
  for (MemoryBuffer *buffer : local_buffers_)
    *const_cast<std::string *>(&buffer->name) =
        struct_var_->name + "->" + buffer->name;
        
  // Now add to the global buffers. 
  for (MemoryBuffer *buffer : global_buffers_)
    *const_cast<std::string *>(&buffer->name) =
        struct_var_->name + "->" + buffer->name;       
}

void Globals::OutputArrayControlVars(std::ostream& out) const {
  size_t max_dimen = Variable::GetMaxArrayDimension(global_vars_);
  std::vector<const Variable *>&ctrl_vars = Variable::get_new_ctrl_vars();
  OutputArrayCtrlVars(ctrl_vars, out, max_dimen, 1);
}

void Globals::HashLocalBuffers(std::ostream& out) const {
  for (MemoryBuffer *buffer : local_buffers_) buffer->hash(out);
}

const Type& Globals::GetGlobalStructPtrType() {
  return *struct_type_ptr_.get();
}

const Variable& Globals::GetGlobalStructVar() {
  return *struct_var_;
}

Globals *Globals::CreateGlobals() {
  return new Globals(*VariableSelector::GetGlobalVariables());
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

void Globals::ModifyGlobalAggregateVariableReferences(Variable *var) {
  assert(var->is_aggregate());
  for (Variable *field_var : var->field_vars) {
    *const_cast<std::string *>(&field_var->name) =
        struct_var_->name + "->" + field_var->name;
    if (field_var->is_aggregate())
      ModifyGlobalAggregateVariableReferences(field_var);
  }
}

}  // namespace CLSmith
