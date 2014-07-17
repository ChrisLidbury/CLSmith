// SIMD expressions.

#ifndef _CLSMITH_EXPRESSIONVECTOR_H_
#define _CLSMITH_EXPRESSIONVECTOR_H_

#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include "CGContext.h"
#include "CLSmith/CLExpression.h"
#include "CLSmith/Vector.h"
#include "CVQualifiers.h"
#include "Type.h"

class Expression;

namespace CLSmith {

class ExpressionVector : public CLExpression {
 public:
  // Type of vector expression.
  enum VectorExprType {
    kLiteral = 0,
    kVariable,
    kSIMD,
    kBuiltIn
  };
  // Type of suffix access.
  enum SuffixAccess {
    kHi = 0,
    kLo,
    kEven,
    kOdd
  };

  ExpressionVector(std::vector<std::unique_ptr<const Expression>>&& exprs,
                   const Type& type, int size, enum SuffixAccess suffix_access)
      : CLExpression(kVector),
        exprs_(std::forward<std::vector<std::unique_ptr<const Expression>>>(exprs)),
        type_(type), size_(size), is_component_access_(false),
        suffix_access_(suffix_access) {
  }
  ExpressionVector(std::vector<std::unique_ptr<const Expression>>&& exprs,
                   const Type& type, int size, std::vector<int> accesses)
      : CLExpression(kVector),
        exprs_(std::forward<std::vector<std::unique_ptr<const Expression>>>(exprs)),
        type_(type), size_(size), is_component_access_(true),
        accesses_(accesses) {
  }
  ExpressionVector(ExpressionVector&& other) = default;
  ExpressionVector& operator=(ExpressionVector&& other) = default;
  virtual ~ExpressionVector() {}

  // Create an expression that produces a vector value.
  // The expression can be a vector literal, vector variable, an operation on
  // one or two vectors or calling a built-in vector function.
  static ExpressionVector *make_random(CGContext &cg_context, const Type *type,
      const CVQualifiers *qfer, int size);

  // Initialise table for selecting vector expression type. Should be called
  // once on start-up.
  static void InitProbabilityTable();

  // Implementations of pure virtual methods in Expression. Most of these are
  // trivial, as the expression evaluates to a runtime constant.
  ExpressionVector *clone() const;
  const Type &get_type() const { return type_; }
  CVQualifiers get_qualifiers() const { return CVQualifiers(true, false); }
  void get_eval_to_subexps(std::vector<const Expression*>& subs) const;
  void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const;
  unsigned get_complexity() const;
  void Output(std::ostream& out) const;

  const std::vector<std::unique_ptr<const Expression>>& GetExpressions() const {
    return exprs_;
  }
 private:
  // Vector of expressions that when combined will form an OpenCL vector.
  std::vector<std::unique_ptr<const Expression>> exprs_;
  // Basic type.
  const Type& type_;
  // Vector length;
  const int size_;
  // Vector accesses.
  bool is_component_access_;  // Is the access per component (i.e. vec.xy).
  std::vector<int> accesses_;  // If it is component accesses, which components.
  enum SuffixAccess suffix_access_;  // If not component, access with suffix.

  DISALLOW_COPY_AND_ASSIGN(ExpressionVector);
};

}  // namespace CLSmith

#endif  // _CLSMITH_EXPRESSIONVECTOR_H_
