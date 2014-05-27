// Expression for retrieving the global id.
// This will add program divergence to wherever this is added.

#ifndef _CLSMITH_EXPRESSIONID_H_
#define _CLSMITH_EXPRESSIONID_H_

#include <fstream>
#include <vector>

#include "CLSmith/CLExpression.h"
#include "CommonMacros.h"
#include "CVQualifiers.h"
#include "Type.h"

namespace CLSmith {

// Translates into a call to get_global_id(0) or get_local_id(0).
// This can be treated mostly as csmith's Constant class, with the exception
// that we don't know the value at compile time.
class ExpressionID : public CLExpression {
 public:
  // Which ID is being requested.
  enum IDType {
    kGlobal = 0,
    kLocal
  };

  explicit ExpressionID(IDType type) : CLExpression(kID),
      id_type_(type),
      type_(Type::get_simple_type(eULongLong)) {
  }
  ExpressionID(ExpressionID&& other) = default;
  ExpressionID& operator=(ExpressionID&& other) = default;
  virtual ~ExpressionID() {}

  static ExpressionID *make_random(const Type* type) {
    assert(type->eType == eSimple && type->simple_type == eULongLong);
    return new ExpressionID(kGlobal);
  }

  // Implementations of pure virtual methods in Expression. Most of these are
  // trivial, as the expression evaluates to a runtime constant.
  Expression *clone() const { return new ExpressionID(id_type_); }
  const Type &get_type() const { return type_; }
  CVQualifiers get_qualifiers() const { return CVQualifiers(true, false); }
  void get_eval_to_subexps(std::vector<const Expression*>& subs) const {
    subs.push_back(this);
  }
  void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const {}
  unsigned get_complexity() const { return 1; }
  void Output(std::ostream& out) const {
    output_cast(out);
    out << (id_type_ == kGlobal ? "get_global_id(0)" : "get_local_id(0)");
  }

  // Both local and global IDs are always divergent.
  bool IsDivergent() const { return true; }

  // Given the context, what is the probability that the expression should be
  // generated.
  static int GetGenerationProbability(
      const CGContext &cg_context, const Type& type) {
    int prob = type.eType == eSimple && type.simple_type == eULongLong ?
        (cg_context.blk_depth + cg_context.expr_depth) / 2 : 0;
    return prob > 5 ? 5 : 0;
  }

 private:
  IDType id_type_;
  const Type& type_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionID);
};

}  // namespace CLSmith

#endif  // _CLSMITH_EXPRESSIONID_H_
