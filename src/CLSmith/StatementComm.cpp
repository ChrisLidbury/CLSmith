#include "CLSmith/StatementComm.h"

#include <algorithm>
#include <ostream>
#include <random>
#include <vector>

#include "Block.h"
#include "CGContext.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/CLProgramGenerator.h" // temp
#include "CLSmith/ExpressionID.h"
#include "CLSmith/Globals.h"
#include "CLSmith/MemoryBuffer.h"
#include "CLSmith/StatementBarrier.h"
#include "Constant.h"
#include "Expression.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "Lhs.h"
#include "SafeOpFlags.h"
#include "StatementAssign.h"
#include "Type.h"
#include "Variable.h"
#include "VariableSelector.h"

namespace CLSmith {
namespace {
// Number of permutations.
const int kPermCount = 10;

// Const buffer that holds the random permutations of [0..31].
MemoryBuffer *permutations;
// Each permutation.
std::vector<int> *permute_values[kPermCount];
// Local buffer that holds the intermediate values.
MemoryBuffer *values;
// Variable that holds the random thread ID.
Variable *tid;
// Variable used in random expressions throughout the program.
MemoryBuffer *var;
}  // namespace

StatementComm *StatementComm::make_random(CGContext& cg_context) {
  // rand_expr
  Expression *expr = Expression::make_random(
      cg_context, &Type::get_simple_type(eUInt));
  // rand_expr % 10
  expr = new ExpressionFuncall(*new FunctionInvocationBinary(eMod,
      expr, Constant::make_int(10), SafeOpFlags::make_dummy_flags()));
  // get_group_id(0) * 32
  Expression *expr_ = new ExpressionFuncall(*new FunctionInvocationBinary(eMul,
      new ExpressionID(ExpressionID::kGroup), Constant::make_int(32),
      SafeOpFlags::make_dummy_flags()));
  // (get_group_id(0) * 32) + tid
  expr_ = new ExpressionFuncall(*new FunctionInvocationBinary(eAdd,
      expr_, new ExpressionVariable(*tid), SafeOpFlags::make_dummy_flags()));
  // permutations[rand_expr % 10][(get_group_id(0) * 32) + tid]
  expr = new ExpressionVariable(*permutations->itemize(
      std::vector<const Expression *>({expr, expr_}),
      cg_context.get_current_block()));
  StatementAssign *st_ass = StatementAssign::make_possible_compound_assign(
      cg_context, *new Lhs(*tid), eSimpleAssign, *expr);
  return new StatementComm(cg_context.get_current_block(),
      new StatementBarrier(cg_context.get_current_block()), st_ass);
}

void StatementComm::InitBuffers() {
  unsigned perm_size = CLProgramGenerator::get_threads_per_group();
  for (int idx = 0; idx < kPermCount; ++idx) {
    permute_values[idx] = new std::vector<int>(perm_size);
    for (unsigned id = 0; id < perm_size; ++id) (*permute_values[idx])[id] = id;
    std::shuffle(permute_values[idx]->begin(), permute_values[idx]->end(),
        std::default_random_engine(rnd_upto(65535)));
  }
  permutations = MemoryBuffer::CreateMemoryBuffer(MemoryBuffer::kConst,
      "permutations", &Type::get_simple_type(eUInt), NULL,
      {kPermCount, perm_size});
  values = MemoryBuffer::CreateMemoryBuffer(MemoryBuffer::kGlobal, "comm_values",
      &Type::get_simple_type(eLongLong), Constant::make_int(1), {32});
  // The initial value of the tid will come from the first permutation.
  // Lots of memory leaks here, probably not worth cleaning.
  ExpressionVariable *init = new ExpressionVariable(*permutations->itemize(  // leak
      std::vector<const Expression *>({
      Constant::make_int(0), new ExpressionID(ExpressionID::kLocal)}),
      new Block(NULL, 0)));  // leak
  tid = Variable::CreateVariable("tid", &Type::get_simple_type(eUInt), init,
      new CVQualifiers(std::vector<bool>({false}), std::vector<bool>({false})));
  // The variable that will be accessed throughout the program randomly.
  var = values->itemize(std::vector<const Expression *>({
      new ExpressionVariable(*tid)}), new Block(NULL, 0));  // leak
  if (CLOptions::inter_thread_comm())
    VariableSelector::GetGlobalVariables()->push_back(var);
}

void StatementComm::OutputPermutations(std::ostream& out) {
  permutations->OutputDecl(out);
  out << " = {" << std::endl;
  for (int idx = 0; idx < kPermCount; ++idx) {
    out << "{" << (*permute_values[idx])[0];
    for (int id = 1; id < (int)permute_values[idx]->size(); ++id)
      out << "," << (*permute_values[idx])[id];
    out << "}";
    if (idx < kPermCount - 1) out << ",";
    out << " // permutation " << idx << std::endl;
  }
  out << "};" << std::endl;
}

void StatementComm::AddVarsToGlobals(Globals *globals) {
  assert(values != NULL && tid != NULL);
  globals->AddGlobalMemoryBuffer(values);
  globals->AddGlobalVariable(tid);
}

void StatementComm::HashCommValues(std::ostream& out) {
  assert(values != NULL);
  values->hash(out);
}

void StatementComm::Output(std::ostream& out, FactMgr *fm, int indent) const {
  barrier_->Output(out, fm, indent);
  assign_->Output(out, fm, indent);
}

}  // namespace CLSmith
