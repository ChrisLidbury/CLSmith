#include "CLSmith/CLExpression.h"

#include "CGContext.h"
#include "CGOptions.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/ExpressionID.h"
#include "CLSmith/ExpressionVector.h"
#include "ProbabilityTable.h"
#include "random.h"
#include "Type.h"
#include "VectorFilter.h"

class CVQualifiers;

namespace CLSmith {
namespace {
DistributionTable *cl_expr_table = NULL;
}  // namespace

/*CL*/Expression *CLExpression::make_random(CGContext &cg_context, const Type *type,
    const CVQualifiers *qfer, enum CLExpressionType tt) {
  // kNone is used for not specifying an expression type, as it does not require
  // recursive generation.
  // If the expected type is a vector type, then automatically set the term type
  // to a vector expression.
  if (tt == kNone && type->eType == eVector) tt = kVector;
  if (tt == kNone) {
    // The probability for Expression picking a CLExpression is the sum of the
    // maximum probabilities of all derived CLExpressions. If CLExpression is
    // picked, a second lookup is performed, possibly returning NULL.
    // Instead of filtering expression types, we decide when they are chosen if
    // they are eligible, if they are not, then return NULL. This prevents
    // overly inflating the probability of being picked.
    assert(cl_expr_table != NULL);
    int num = rnd_upto(15);
    tt = type->eType == eVector ? kVector :
        (CLExpressionType)VectorFilter(cl_expr_table).lookup(num);
    // kID will be one of divergence, fake divergence or group divergence.
    // Probability of calling get_global_id(0) without faking divergence is
    // dependent on block and expression depth.
    if (tt == kID) {
      if (!CLOptions::divergence() && !CLOptions::fake_divergence() &&
          !CLOptions::group_divergence())
        return NULL;
      if (type->eType != eSimple || type->is_signed())
        return NULL;
      // TODO find a way to limit div like this.
      //if (CLOptions::divergence() && !CLOptions::fake_divergence() &&
      //    !CLOptions::group_divergence() &&
      //    (unsigned)ExpressionID::GetGenerationProbability(cg_context, *type) <=
      //    rnd_upto(5))
    }
    // Not many restrictions on vector types.
    if (tt == kVector) {
      if (!CLOptions::vectors() ||
          (type->eType != eSimple && type->eType != eVector) ||
          cg_context.expr_depth + 2 > CGOptions::max_expr_depth())
        return NULL;
    }
  }

  /*CL*/Expression *expr = NULL;
  switch (tt) {
    case kID:
      expr = ExpressionID::make_random(cg_context, type); break;
    case kVector:
      expr = ExpressionVector::make_random(cg_context, type, qfer, 0); break;
    default: assert(false);
  }
  return expr;
}

void CLExpression::InitProbabilityTable() {
  // All probabilities are added, even if the expression type is disabled. This
  // is because the probability of picking a CLExpression is fixed in
  // Expression, so not adding them would artificially increase the
  // probabilities of other CLExpressions much more than desired.
  cl_expr_table = new DistributionTable();
  cl_expr_table->add_entry(kID, 5);
  cl_expr_table->add_entry(kVector, 10);
  ExpressionVector::InitProbabilityTable();
}

Expression *make_random(CGContext &cg_context, const Type *type,
    const CVQualifiers *qfer) {
  return CLExpression::make_random(cg_context, type, qfer, CLExpression::kNone);
}

}  // namespace CLSmith
