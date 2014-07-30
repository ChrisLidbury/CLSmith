// Expression for hanling atomic operations in OpenCL
// Currently, only generated as if condition
// TODO

#ifndef _CLSMITH_EXPRESSIONATOMIC_H_
#define _CLSMITH_EXPRESSIONATOMIC_H_

#include "CLSmith/CLExpression.h"
#include "CLSmith/MemoryBuffer.h"

#include "CGContext.h"
#include "Type.h"
#include "ArrayVariable.h"

namespace CLSmith {
  
class ExpressionAtomic : public CLExpression {
 public:
  // Atomic expression type
  enum AtomicExprType {
    kAdd = 0,
    kSub,
    kXchg,
    kInc,
    kDec,
    kCmpxchg,
    kMin,
    kMax, 
    kAnd,
    kOr,
    kXor
  };
  
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var) : CLExpression(kAtomic), 
    global_var_(global_var), atomic_type_(type), type_(Type::get_simple_type(eInt)) {}
  
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var, int val) : 
    CLExpression(kAtomic), global_var_(global_var), val_(val), atomic_type_(type), 
    type_(Type::get_simple_type(eInt)) {}
    
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var, int val, int cmp) : 
    CLExpression(kAtomic), global_var_(global_var), val_(val), cmp_(cmp), 
    atomic_type_(type), type_(Type::get_simple_type(eInt)) {}
    
  static ExpressionAtomic *make_random(CGContext &cg_context, const Type *type);
  static MemoryBuffer* GetGlobalBuffer();
  static std::vector<MemoryBuffer*>* GetGlobalMems();
  
      
    
  // Pure virtual methods from Expression
  ExpressionAtomic *clone() const { return NULL; };
  const Type &get_type() const { return type_; };
  CVQualifiers get_qualifiers() const { return CVQualifiers(true, false); }
  void get_eval_to_subexps(std::vector<const Expression*>& subs) const {}
  void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const {}
  unsigned get_complexity() const { return 1; }

  
  void Output(std::ostream& out) const;
  
    
 private:
  ArrayVariable* global_var_;
  int val_;
  int cmp_;
  AtomicExprType atomic_type_;
  const Type& type_;
  
  //TODO make these variables?
  
  string GetOpName() const;
  
  DISALLOW_COPY_AND_ASSIGN(ExpressionAtomic);
};

} // namespace CLSmith

#endif // _CLSMITH_EXPRESSIONATOMIC_H_