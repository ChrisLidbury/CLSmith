#include "CLSmith/ExpressionID.h"

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "ArrayVariable.h"
#include "Block.h"
#include "CGContext.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/Globals.h"
#include "CLSmith/MemoryBuffer.h"
#include "CVQualifiers.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationBinary.h"
#include "random.h"
#include "SafeOpFlags.h"
#include "Type.h"
#include "Variable.h"

namespace CLSmith {
namespace {
MemoryBuffer *sequence_input;
Variable *offsets[9];
}  // namespace

Expression/*ID*/ *ExpressionID::make_random(CGContext& cg_context,
    const Type* type) {
  assert(type->eType == eSimple && !type->is_signed());
  enum GenType { kDiv, kGrp, kFake };
  std::vector<enum GenType> selector;
  if (CLOptions::divergence()) selector.push_back(kDiv);
  if (CLOptions::group_divergence()) selector.push_back(kGrp);
  if (CLOptions::fake_divergence()) selector.push_back(kFake);
  assert(selector.size() > 0);
  enum GenType gen = selector[rnd_upto(selector.size())];
  switch (gen) {
    case kDiv: return new ExpressionID(kGlobal);
    case kGrp: return new ExpressionID(kGroup, rnd_upto(3));
    case kFake:
        return ExpressionID::CreateFakeDivergentExpression(cg_context, *type);
  }
  assert(false && "Should not reach here.");
}

void ExpressionID::Initialise() {
  if (sequence_input != NULL) return;
  const Type *utype = &Type::get_simple_type(eULongLong);
  //CVQualifiers *cv = new CVQualifiers(false, false);
  // Required due to derpy itemise. Will never be free'd.
  Block *blk = new Block(NULL, 0);

  sequence_input = MemoryBuffer::CreateMemoryBuffer(
      MemoryBuffer::kGlobal, "sequence_input", utype, NULL, {1024});
  for (int idx = 0; idx < 3; ++idx)
    for (int id = kGlobal; id <= kGroup; ++id) {
      CVQualifiers *cv = new CVQualifiers(
          std::vector<bool>({false}), std::vector<bool>({false}));
      stringstream ss;
      OutputIDType(ss, (IDType)id);
      ss << '_' << idx << "_offset";
      ArrayVariable *item = sequence_input->itemize(
          std::vector<const Expression *>({new ExpressionID((IDType)id, idx)}),
          blk);
      offsets[id * 3 + idx] = Variable::CreateVariable(
          ss.str(), utype, new ExpressionVariable(*item), cv);
  }
}

void ExpressionID::AddVarsToGlobals(Globals *globals) {
  for (int idx = 0; idx < 9; ++idx)
    globals->AddGlobalVariable(offsets[idx]);
}

int ExpressionID::GetGenerationProbability(
    const CGContext &cg_context, const Type& type) {
  int prob = type.eType == eSimple && type.simple_type == eULongLong ?
      (cg_context.blk_depth + cg_context.expr_depth) / 2 : 0;
  return prob > 5 ? 5 : 0;
}

Expression *ExpressionID::CreateFakeDivergentExpression(
    const CGContext& cg_context, const Type& type) {
  assert(sequence_input != NULL);
  if (type.eType != eSimple || type.is_signed()) return NULL;
  IDType id = (IDType)rnd_upto(3);
  int dim = rnd_upto(3);
  const Variable *var = offsets[id * 3 + dim];
  return new ExpressionFuncall(*new FunctionInvocationBinary(eSub,
      new ExpressionVariable(*var), new ExpressionID(id, dim),
      SafeOpFlags::make_dummy_flags()));
}

void ExpressionID::OutputIDType(std::ostream& out, IDType id_type) {
  switch (id_type) {
    case kGlobal:       out << "global";        return;
    case kLocal:        out << "local";         return;
    case kGroup:        out << "group";         return;
    case kLinearGlobal: out << "linear_global"; return;
    case kLinearLocal:  out << "linear_local";  return;
    case kLinearGroup:  out << "linear_group";  return;
  }
}

void ExpressionID::Output(std::ostream& out) const {
  output_cast(out);
  out << "get_";
  OutputIDType(out, id_type_);
  out << "_id(";
  if (id_type_ <= kGroup) out << dimension_;
  out << ")";
}

}  // namespace CLSmith
