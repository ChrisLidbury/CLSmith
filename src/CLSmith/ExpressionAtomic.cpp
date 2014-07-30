#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/Globals.h"

#include <vector>

#include "Expression.h"
#include "CGContext.h"
#include "random.h"
#include "ArrayVariable.h"
#include "Constant.h"
#include "VariableSelector.h"


namespace CLSmith {
namespace {
static MemoryBuffer* global_buf = NULL;
static std::vector<MemoryBuffer*>* global_mems = NULL;
} // namespace
  
ExpressionAtomic* ExpressionAtomic::make_random(CGContext &cg_context, const Type *type) {
  assert(type->eType == eSimple && type->simple_type == eInt);
  std::vector<int> rand_index (1, rnd_upto(GetGlobalBuffer()->get_size()));
  MemoryBuffer* itemized = GetGlobalBuffer()->itemize(rand_index);
  GetGlobalMems()->push_back(itemized);
  AtomicExprType atomic_type = (AtomicExprType) rnd_upto(kXor);
  switch(atomic_type) {
    case kDec:
    case kInc:
      return new ExpressionAtomic(atomic_type, itemized);
    case kAdd:
    case kSub:
    case kXchg:
    case kMin:
    case kMax:
    case kAnd:
    case kOr:
    case kXor: 
      return new ExpressionAtomic(atomic_type, itemized, rnd_upto(100));
    case kCmpxchg:
      return new ExpressionAtomic(atomic_type, itemized, rnd_upto(100), 0);
    default: assert(0); //TODO throw error
  }
}

MemoryBuffer* ExpressionAtomic::GetGlobalBuffer() {
  if (global_buf == NULL)  {
    global_buf = MemoryBuffer::CreateMemoryBuffer(MemoryBuffer::kGlobal,
      "input", &Type::get_simple_type(eInt), NULL, 1024);
    GetGlobalMems()->push_back(global_buf);
  }
  return global_buf;
}

std::vector<MemoryBuffer*>* ExpressionAtomic::GetGlobalMems() {
  if (global_mems == NULL)
    global_mems = new std::vector<MemoryBuffer*>;
  return global_mems;
}

void ExpressionAtomic::Output(std::ostream& out) const {
  out << GetOpName() << "(";
  global_var_->Output(out);
  switch(atomic_type_) {
    case kDec:
    case kInc: break;
    case kAdd:
    case kSub:
    case kXchg:
    case kMin:
    case kMax:
    case kAnd:
    case kOr:
    case kXor: { out << ", " << val_; break; }
    case kCmpxchg: { out << ", " << cmp_ << ", " << val_; break; }  
  }
  out << ")";
}

string ExpressionAtomic::GetOpName() const {
  switch(atomic_type_) {
    case kAdd:     return "atomic_add";
    case kSub:     return "atomic_sub";
    case kXchg:    return "atomic_xchg";
    case kInc:     return "atomic_inc";
    case kDec:     return "atomic_dec";
    case kCmpxchg: return "atomic_cmpxchg";
    case kMin:     return "atomic_min";
    case kMax:     return "atomic_max";
    case kAnd:     return "atomic_and";
    case kOr:      return "atomic_or";
    case kXor:     return "atomic_xor";
    default:       assert(0); //TODO throw error 
  }
}
}