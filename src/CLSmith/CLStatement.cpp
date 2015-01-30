#include "CLSmith/CLStatement.h"

#include "Block.h"
#include "CGContext.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/ExpressionID.h"
#include "CLSmith/StatementComm.h"
#include "CLSmith/StatementAtomicReduction.h"
#include "CLSmith/StatementEMI.h"
#include "ProbabilityTable.h"
#include "Type.h"
#include "VectorFilter.h"

namespace CLSmith {
namespace {
DistributionTable *cl_stmt_table = NULL;
}  // namespace

CLStatement *CLStatement::make_random(CGContext& cg_context,
    enum CLStatementType st) {
  // kNone means no statement type has been specified. A statement type will be
  // randomly chosen.
  if (st == kNone) {
    // Statement generation works mostly the same as expressions. The exception
    // is that if NULL is returned to Statement::make_random(), it will
    // recursively call itself, instead of specifying a non CLStatement.
    assert (cl_stmt_table != NULL);
    int num = rnd_upto(25);
    st = (CLStatementType)VectorFilter(cl_stmt_table).lookup(num);
    // Must only include barriers if they are enabled and no divergence.
    if (st == kBarrier) {
      if (!CLOptions::barriers() || CLOptions::divergence()) return NULL;
    }
    // Only use EMI blocks if they are enabled and we are not already in one.
    if (st == kEMI) {
      if (!CLOptions::emi() || cg_context.get_emi_context()) return NULL;
    }
    // Only generate atomic reductions if they are set.
    if (st == kReduction) {
      if (!CLOptions::atomic_reductions() || cg_context.get_atomic_context())
        return NULL;
    }
    // Fake divergence must also be set.
    // Not currently used, as fake divergence is handled by ExpressionID, which
    // will sometimes produce an expression using get_global_id. Would like to
    // move that functionality here for more control.
    if (st == kFakeDiverge) {
      /*if (!CLOptions::fake_divergence())*/ return NULL;
    }
    // Inter-thread communication must be set.
    if (st == kComm) {
      if (!CLOptions::inter_thread_comm() || cg_context.get_atomic_context())
        return NULL;
    }
  }

  CLStatement *stmt = NULL;
  switch (st) {
    case kBarrier:
      assert(false);
    case kEMI:
      stmt = StatementEMI::make_random(cg_context); break;
    case kReduction:
      stmt = StatementAtomicReduction::make_random(cg_context); break;
//    case kFakeDiverge:
//      stmt = CreateFakeDivergentIf(cg_context); break;
    case kComm:
      stmt = StatementComm::make_random(cg_context); break;
    default: assert(false);
  }
  return stmt;
}

/*CLStatement *CLStatement::CreateFakeDivergentIf(CGContext& cg_context) {
  const Type& type = Type::get_simple_type(eULongLong);
  StatementIf *st_if = new StatementIf(cg_context.get_current_block(),
      *ExpressionID::CreateFakeDivergentExpression(cg_context, type),
      *Block::make_random(cg_context), *Block::make_random(cg_context));
  // For now, bundle it in EMI but do not register with the controller.
  StatementEMI *cl_st = new StatementEMI(cg_context.get_current_block(), st_if);
  return cl_st;
}*/

void CLStatement::InitProbabilityTable() {
  cl_stmt_table = new DistributionTable();
  cl_stmt_table->add_entry(kBarrier, 5);
  cl_stmt_table->add_entry(kEMI, 5);
  cl_stmt_table->add_entry(kReduction, 5);
  cl_stmt_table->add_entry(kFakeDiverge, 5);
  cl_stmt_table->add_entry(kComm, 5);
}

Statement *make_random_st(CGContext& cg_context) {
  return CLStatement::make_random(cg_context, CLStatement::kNone);
}

}  // namespace CLSmith
