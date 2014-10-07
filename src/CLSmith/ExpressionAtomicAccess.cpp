#include "CLSmith/ExpressionAtomicAccess.h"
#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/ExpressionID.h"

namespace CLSmith {
  
void ExpressionAtomicAccess::Output(std::ostream& out) const {
  if (mem_ == kGlobal) {
    out << ExpressionAtomic::get_atomic_blocks_no() << " * ";
    ExpressionID* g_access = new ExpressionID(ExpressionID::kLinearGroup);
    g_access->Output(out);
    out << " + " << constant_;
  } else if (mem_ == kLocal) {
    out << constant_;
  }
}

bool ExpressionAtomicAccess::is_global() const {
  return mem_ == kGlobal;
}

bool ExpressionAtomicAccess::is_local() const {
  return mem_ == kLocal;
}

}