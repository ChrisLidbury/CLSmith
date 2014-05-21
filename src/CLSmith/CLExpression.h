// Root of expressions specific to OpenCL.
// Acts as a hook by which CLSmith can inject behaviour into csmith.

#ifndef _CLSMITH_CLEXPRESSION_H_
#define _CLSMITH_CLEXPRESSION_H_

#include "CommonMacros.h"
#include "Expression.h"

class CGContext;
class Type;
class VectorFilter;

namespace CLSmith {

// All OpenCL related expressions derive from this.
class CLExpression : public Expression {
 public:
  // Dynamic type information.
  enum CLExpressionType {
    kID = 0
  };

  explicit CLExpression(CLExpressionType type) : Expression(eCLExpression),
      cl_expression_type_(type) {
  }
  CLExpression(CLExpression&& other) = default;
  CLExpression& operator=(CLExpression&& other) = default;
  virtual ~CLExpression() {}

  static CLExpression *make_random(CGContext &cg_context, const Type *type);

  // When evaluated at runtime, will this produce a divergent value.
  virtual bool IsDivergent() const = 0;

 private:
  CLExpressionType cl_expression_type_;

  DISALLOW_COPY_AND_ASSIGN(CLExpression);
};

// Hook method for csmith.
Expression *make_random(CGContext &cg_context, const Type *type) { 
  return CLExpression::make_random(cg_context, type);
}

}  // namespace CLSmith

#endif  // _CLSMITH_CLEXPRESSION_H_
