// Root of expressions specific to OpenCL.
// Acts as a hook by which CLSmith can inject behaviour into csmith.

#ifndef _CLSMITH_CLEXPRESSION_H_
#define _CLSMITH_CLEXPRESSION_H_

#include "CommonMacros.h"
#include "Expression.h"

class CGContext;
class CVQualifiers;
class DistributionTable;
class Type;
class VectorFilter;

namespace CLSmith {

// All OpenCL related expressions derive from this.
// The type of OpenCL expression to use is handled here, instead of in
// Expression. So we have our own probability table.
class CLExpression : public Expression {
 public:
  // Dynamic type information.
  enum CLExpressionType {
    kNone = 0,  // Sentinel value.
    kID,
    kVector,
    kAtomic
  };

  explicit CLExpression(CLExpressionType type) : Expression(eCLExpression),
      cl_expression_type_(type) {
  }
  // Must be careful with the move constructors. These will call the copy constructor
  // in Expression.
  CLExpression(CLExpression&& other) = default;
  CLExpression& operator=(CLExpression&& other) = default;
  virtual ~CLExpression() {}

  // Factory for creating a random OpenCL expression. This will typically be
  // called by Expression::make_random.
  static CLExpression *make_random(CGContext &cg_context, const Type *type,
      const CVQualifiers* qfer, enum CLExpressionType tt);

  // Create the random probability table, should be called once on startup.
  static void InitProbabilityTable();

  // When evaluated at runtime, will this produce a divergent value.
  virtual bool IsDivergent() const { return false; }//; = 0;
  
  // Getter for cl_expression_type_
  enum CLExpressionType GetCLExpressionType() const {
    return cl_expression_type_;
  }

 private:
  CLExpressionType cl_expression_type_;
  // Table used for deciding on which OpenCL expression to generate.
  //static DistributionTable *cl_expr_table_;

  DISALLOW_COPY_AND_ASSIGN(CLExpression);
};

// Hook method for csmith.
Expression *make_random(CGContext& cg_context, const Type *type,
    const CVQualifiers *qfer);

}  // namespace CLSmith

#endif  // _CLSMITH_CLEXPRESSION_H_
