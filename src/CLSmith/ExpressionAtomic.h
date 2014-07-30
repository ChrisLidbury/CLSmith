// Expression for hanling atomic operations in OpenCL
// Currently, only generated as if condition
//
// TODO
// * generate code inside the then / else blocks
// * make a verification mechanism

#ifndef _CLSMITH_EXPRESSIONATOMIC_H_
#define _CLSMITH_EXPRESSIONATOMIC_H_

#include "CLSmith/CLExpression.h"
#include "CLSmith/MemoryBuffer.h"

#include "ArrayVariable.h"
#include "CGContext.h"
#include "Type.h"

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
  
  // Three constructors for the three possible operations:
  // * inc and dec have one parameter;
  // * cmpxchg has three parameters;
  // * other operations have two parameters.
  // For simplicity, all the parameters are considered int, coming from a
  // __global volatile array passed as a parameter to the kernel.
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var) : CLExpression(kAtomic), 
    global_var_(global_var), atomic_type_(type), type_(Type::get_simple_type(eInt)) {}
  
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var, int val) : 
    CLExpression(kAtomic), global_var_(global_var), val_(val), atomic_type_(type), 
    type_(Type::get_simple_type(eInt)) {}
    
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var, int val, int cmp) : 
    CLExpression(kAtomic), global_var_(global_var), val_(val), cmp_(cmp), 
    atomic_type_(type), type_(Type::get_simple_type(eInt)) {}
  
  // Generates a random atomic expression; all expressions have equal chance of 
  // being generated.
  static ExpressionAtomic *make_random(CGContext &cg_context, const Type *type);
  
  // This represents the __global volatile array parameter from the kernel;
  // the underlying object is a singleton MemoryBuffer.
  static MemoryBuffer* GetGlobalBuffer();
  
  // Since all the p arguments of the atomic expressions come from the global
  // array, it must be held in the struct, for global access. Here we save all
  // array accesses, which we will add after the program generation to the 
  // global struct; this is required as the global struct should not get
  // generated until after the entire program is complete.
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
  // Represents the p parameter of the expression; it is an array access into
  // the global array parameter
  ArrayVariable* global_var_;
  
  // The two additional parameters required by certain operations
  // TODO consider making Variable
  int val_;
  int cmp_;
  
  // The type of the expression, one for CLSmith and the other for CSmith
  AtomicExprType atomic_type_;
  const Type& type_;
  
  // Helper function returning the operation name for the Output() function
  string GetOpName() const;
  
  DISALLOW_COPY_AND_ASSIGN(ExpressionAtomic);
};

} // namespace CLSmith

#endif // _CLSMITH_EXPRESSIONATOMIC_H_