#ifndef _CLSMITH_EXPRESSIONATOMICACCESS_H_
#define _CLSMITH_EXPRESSIONATOMICACCESS_H_

#include "CLSmith/CLExpression.h"

#include "Expression.h"
#include "Type.h"

namespace CLSmith {

class ExpressionAtomicAccess : public CLExpression {
 public:
  enum AtomicMemAccess {
    kGlobal = 0,
    kLocal
  };
   
  ExpressionAtomicAccess (const int constant, const AtomicMemAccess mem) : CLExpression(kAtomic),
    constant_(constant), mem_(mem), type_(Type::get_simple_type(eInt)) {}
    
  // TODO
  bool is_global(void) const;
  bool is_local(void) const;
    
  // TODO
  int get_counter(void) const {return this->constant_;};
  
  // Pure virtual methods from Expression
  ExpressionAtomicAccess *clone() const { return NULL; };
  const Type &get_type() const { return type_; };
  CVQualifiers get_qualifiers() const { return CVQualifiers(true, false); }
  void get_eval_to_subexps(std::vector<const Expression*>& subs) const {}
  void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const {}
  unsigned get_complexity() const { return 1; }

  void Output(std::ostream& out) const;
  
 private:
  const int constant_;
  const AtomicMemAccess mem_;
  
//   Expression::AtomicMemory mem_;
  const Type& type_;
  
  DISALLOW_COPY_AND_ASSIGN(ExpressionAtomicAccess);
};

}

#endif  //  _CLSMITH_EXPRESSIONATOMICACCESS_H_