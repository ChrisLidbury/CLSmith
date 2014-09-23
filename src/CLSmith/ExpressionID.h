// Expression for retrieving the global id.
// This will add program divergence to wherever this is added.
// Also handles the generation of expressions involving the use of ID, such as
// faking divergence.

#ifndef _CLSMITH_EXPRESSIONID_H_
#define _CLSMITH_EXPRESSIONID_H_

#include <ostream>
#include <vector>

#include "CLSmith/CLExpression.h"
#include "CommonMacros.h"
#include "CVQualifiers.h"
#include "Type.h"

namespace CLSmith { class Globals; }
class CGContext;
class Expression;

namespace CLSmith {

// Translates into a call to get_global_id(0) or get_local_id(0).
// This can be treated mostly as csmith's Constant class, with the exception
// that we don't know the value at compile time.
class ExpressionID : public CLExpression {
 public:
  // Which ID is being requested.
  enum IDType {
    kGlobal = 0,
    kLocal,
    kGroup
  };

  explicit ExpressionID(IDType type) : CLExpression(kID),
      id_type_(type), dimension_(0),
      type_(Type::get_simple_type(eULongLong)) {
  }
  ExpressionID(IDType type, int dimension) : CLExpression(kID),
      id_type_(type), dimension_(dimension),
      type_(Type::get_simple_type(eULongLong)) {
  }
  ExpressionID(ExpressionID&& other) = default;
  ExpressionID& operator=(ExpressionID&& other) = default;
  virtual ~ExpressionID() {}

  // Will just return a call to get_global_id(0).
  static ExpressionID *make_random(const Type* type);

  // Initialise the static data used for divergence faking.
  static void Initialise();

  // Add any variables referred to throughout the program to the globals struct.
  static void AddVarsToGlobals(Globals *globals);

  // Given the context, what is the probability that the expression should be
  // generated.
  static int GetGenerationProbability(
      const CGContext &cg_context, const Type& type);

  // Create an expression that has uniform value across work items, but the
  // compiler assumes is non-uniform.
  static Expression *CreateFakeDivergentExpression(
      const CGContext& cg_context, const Type& type);

  // Outputs the name of the type of identifier (e.g. 'local' for get_local_id).
  static void OutputIDType(std::ostream& out, IDType id_type);

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
  void Output(std::ostream& out) const;

  // Both local and global IDs are always divergent.
  bool IsDivergent() const { return true; }

 private:
  IDType id_type_;
  int dimension_;
  const Type& type_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionID);
};

}  // namespace CLSmith

#endif  // _CLSMITH_EXPRESSIONID_H_
