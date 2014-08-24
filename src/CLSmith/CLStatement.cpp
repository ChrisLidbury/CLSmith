#include "CLSmith/CLStatement.h"

#include "CGContext.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/StatementEMI.h"

namespace CLSmith {

void CLStatement::InitProbabilityTable() {
  // TODO when there are more statements. EMI will be 5%.
}

CLStatement *CLStatement::make_random(CGContext& cg_context,
    enum CLStatementType st) {
  // kNone means no statement type has been specified. A statement type will be
  // randomly chosen.
  if (st == kNone) {
    // TODO random st, just use EMI for now.
    // Statement generation works mostly the same as expressions. The exception
    // is that if NULL is returned to Statement::make_random(), it will
    // recursively call itself, instead of specifying a non CLStatement.
    st = kEMI;
    // Must only include barriers if they are enabled and no divergence.
    if (st == kBarrier) {
      if (!CLOptions::barriers() || CLOptions::divergence()) return NULL;
    }
    // Only use EMI blocks if they are enabled and we are not already in one.
    if (st == kEMI) {
      if (!CLOptions::EMI() || cg_context.get_emi_context()) return NULL;
    }
  }

  CLStatement *stmt = NULL;
  switch (st) {
    case kBarrier:
      assert(false);
    case kEMI:
      stmt = StatementEMI::make_random(cg_context); break;
    default: assert(false);
  }
  return stmt;
}

Statement *make_random_st(CGContext& cg_context) {
  return CLStatement::make_random(cg_context, CLStatement::kNone);
}

}  // namespace CLSmith
