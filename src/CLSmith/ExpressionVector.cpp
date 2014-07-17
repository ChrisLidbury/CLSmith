#include "CLSmith/ExpressionVector.h"

#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include "CLSmith/CLExpression.h"
#include "Constant.h"
#include "CGContext.h"
#include "Expression.h"
#include "ProbabilityTable.h"
#include "random.h"
#include "Type.h"
#include "VectorFilter.h"

namespace CLSmith {
namespace {
DistributionTable *vector_expr_table = NULL;
DistributionTable *suffix_table = NULL;
}  // namespace

ExpressionVector *ExpressionVector::make_random(CGContext &cg_context,
    const Type *type, const CVQualifiers *qfer, int size) {
  // Check the type. If type is not a vector type, then the expression should
  // produce a single value, so the result must be itemised. If type is a
  // vector type, the size of the vector may be different to the passed size.
  if (!size) size = Vector::GetRandomVectorLength(0);
  int itemise_to = type->eType != eVector ? 1 : type->vector_length_;

  assert(vector_expr_table != NULL);
  int num = rnd_upto(40);
  //enum VectorExprType vec_expr_type =
  //    (VectorExprType)VectorFilter(vector_expr_table).lookup(num);

  // Create a series of sub-expressions. The size of each sub-expression should
  // combine to form the size of the vector.
  std::vector<std::unique_ptr<const Expression>> exprs;
  //if (vec_expr_type == kLiteral) {
    // Randomly create elements equal to size.
    int remain = size;
    while (remain > 0) {
      num = rnd_upto(30);
      if (remain == 1 || num < 10) {
        // Create a simple constant value.
        //exprs.emplace_back(Constant::make_random(
        //    &Vector::DemoteVectorTypeToType(type)));
        exprs.emplace_back(Constant::make_int(69));
        --remain;
      } else if (num < 20) {
        // Create a sub-vector.
        int vec_size = Vector::GetRandomVectorLength(remain);
        const Type *sub_vec_type =
            Vector::PromoteTypeToVectorType(type, vec_size);  // memory leak
        exprs.emplace_back(ExpressionVector::make_random(
            cg_context, sub_vec_type, qfer, vec_size));
        remain -= vec_size;
      } else {
        // Completely random other expression.
        const Type *simple_type = &Vector::DemoteVectorTypeToType(type);
        exprs.emplace_back(
            Expression::make_random(cg_context, simple_type, qfer));
        --remain;
      }
    }
  /*}
  if (vec_expr_type == kVariable || vec_expr_type == kBuiltIn) {
    // Produce an entire vector.
    vec_expr_type = kSIMD; // TODO
  }
  if (vec_expr_type == kSIMD) {
    // Produce an expression that performs a series of operations on vectors. We
    // borrow from csmith's expression generation where possible.
    exprs.emplace_back(Expression::make_random(
        cg_context, vec_type, qfer, false, false, eFunction));
  }*/

  // The expression has been produced, but we need to itemise according to the
  // size of the expected type.
  ExpressionVector *expr_vec = NULL;
  // The expression type when queried will be the basic type.
  //const Type& expr_type = Vector::DemoteVectorTypeToType(vec_type);
  if (size == itemise_to * 2) {
    // Use a suffix if the expected vector length is half of the produced vec.
    assert(suffix_table != NULL);
    num = rnd_upto(40);
    enum SuffixAccess suffix =
        (SuffixAccess)VectorFilter(suffix_table).lookup(num);
    expr_vec = new ExpressionVector(std::move(exprs), /*expr_*/*type, size, suffix);
  } else {
    // Create a series of random accesses.
    std::vector<int> accesses;
    int remain = size == itemise_to ? itemise_to : 0;
    for (; remain < itemise_to; ++remain) accesses.push_back(rnd_upto(size));
    expr_vec = new ExpressionVector(std::move(exprs), /*expr_*/*type, size, accesses);
  }
  //if (vec_type != type) delete vec_type;
  return expr_vec;
}

void ExpressionVector::InitProbabilityTable() {
  vector_expr_table = new DistributionTable();
  vector_expr_table->add_entry(kLiteral, 10);
  vector_expr_table->add_entry(kVector, 10);
  vector_expr_table->add_entry(kSIMD, 10);
  vector_expr_table->add_entry(kBuiltIn, 10);
  suffix_table = new DistributionTable();
  suffix_table->add_entry(kHi, 10);
  suffix_table->add_entry(kLo, 10);
  suffix_table->add_entry(kEven, 10);
  suffix_table->add_entry(kOdd, 10);
}

ExpressionVector *ExpressionVector::clone() const {
  std::vector<std::unique_ptr<const Expression>> exprs;
  for (const auto& expr : exprs_) exprs.emplace_back(expr->clone());
  return is_component_access_ ?
      new ExpressionVector(std::move(exprs), type_, size_, accesses_) :
      new ExpressionVector(std::move(exprs), type_, size_, suffix_access_);
}

void ExpressionVector::get_eval_to_subexps(std::vector<const Expression*>& subs)
    const {
  subs.push_back(this);
  for (const std::unique_ptr<const Expression>& expr : exprs_)
    expr->get_eval_to_subexps(subs);
}

void ExpressionVector::get_referenced_ptrs(std::vector<const Variable*>& ptrs)
    const {
  for (const std::unique_ptr<const Expression>& expr : exprs_)
    expr->get_referenced_ptrs(ptrs);
}

unsigned ExpressionVector::get_complexity() const {
  unsigned complexity = 1;
  for (const std::unique_ptr<const Expression>& expr : exprs_)
    complexity += expr->get_complexity();
  return complexity;
}

// TODO let Vector handle this.
void ExpressionVector::Output(std::ostream& out) const {
  if (exprs_.size() > 1) {
    out << '(';
    Vector::OutputVectorType(out, &type_, size_);
    out << ")(";
  }
  for (unsigned idx = 0; idx < exprs_.size(); ++idx) {
    exprs_[idx]->Output(out);
    if (exprs_.size() - idx > 1) out << ", ";
  }
  if (exprs_.size() > 1) out << ')';
  // Output accesses if any.
  if (is_component_access_ && accesses_.empty()) return;
  out << '.';
  if (!is_component_access_) {
    out << Vector::GetSuffixString(suffix_access_);
  } else {
    if (size_ > 4) out << 's';
    for (int access : accesses_) out << Vector::GetComponentChar(size_, access);
  }
}

}  // namespace CLSmith
