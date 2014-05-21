#include "CLSmith/CLExpression.h"

#include "CLSmith/ExpressionID.h"
#include "random.h"

class CGContext;
class Type;

namespace CLSmith {

CLExpression *CLExpression::make_random(
    CGContext &cg_context, const Type *type) {
  // The probability for Expression picking a CLExpression is the sum of the
  // maximum probabilities of all derived CLExpressions. If CLExpression is
  // picked, a second lookup is performed, possibly returning NULL.
  int rnd_number = rnd_upto(5);
  int expr_id_prob = ExpressionID::GetGenerationProbability(cg_context, *type);
  return expr_id_prob > rnd_number ? ExpressionID::make_random(type) : NULL;
}

Expression *make_random(CGContext &cg_context, const Type *type) { 
  return CLExpression::make_random(cg_context, type);
}

}  // namespace CLSmith
