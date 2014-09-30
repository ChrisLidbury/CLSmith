#include "CLSmith/ExpressionAtomicAccess.h"
#include "CLSmith/ExpressionAtomic.h"

namespace CLSmith {
  
void ExpressionAtomicAccess::Output(std::ostream& out) const {
  if (mem_ == kGlobal) {
    out << ExpressionAtomic::get_atomic_blocks_no() << " * linear_group_id() + " 
      << constant_;
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