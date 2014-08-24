#include "CLSmith/StatementEMI.h"

#include <vector>

#include "Block.h"
#include "CGContext.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/MemoryBuffer.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "FunctionInvocationBinary.h"
#include "SafeOpFlags.h"
#include "Statement.h"
#include "StatementFor.h"
#include "StatementIf.h"

namespace CLSmith {
namespace {
EMIController *emi_controller_inst = NULL;  // Singleton instance.
}  // namespace

StatementEMI *StatementEMI::make_random(CGContext& cg_context) {
  // TODO, better exprs, for now, just do 0>1, 2>3, etc.
  static int item_count = 0;
  assert(item_count < 1024);
  MemoryBuffer *emi_input = EMIController::GetEMIController()->GetEMIInput();
  MemoryBuffer *item1 = emi_input->itemize({item_count++});
  MemoryBuffer *item2 = emi_input->itemize({item_count++});
  SafeOpFlags *flags = SafeOpFlags::make_dummy_flags();
  Expression *test = new ExpressionFuncall(*new FunctionInvocationBinary(eCmpLt,
      new ExpressionVariable(*item1), new ExpressionVariable(*item2), flags));
  // True block is a standard random block (that will never run), no false block.
  // TODO make separate context, and discard effects (it is never executed).
  bool prev_emi_context = cg_context.get_emi_context();
  cg_context.set_emi_context(true);
  Block *true_block = Block::make_random(cg_context);
  Block *false_block = Block::make_dummy_block(cg_context);
  cg_context.set_emi_context(prev_emi_context);
  StatementEMI *emi = new StatementEMI(
      cg_context.get_current_block(), new StatementIf(
      cg_context.get_current_block(), *test, *true_block, *false_block));
  // Record information in the controller.
  EMIController *emi_controller = EMIController::GetEMIController();
  emi_controller->AddStatementEMI(emi);
  emi_controller->AddItemisedEMIInput(item1);
  emi_controller->AddItemisedEMIInput(item2);
  return emi;
}

void StatementEMI::Prune() {
  // 'const' pfffft
  PruneBlock(const_cast<Block *>(if_block_->get_true_branch()));
}

void StatementEMI::PruneBlock(Block *block) {
  std::vector<Statement *> del_stms;
  for (Statement *st : block->stms) {
    eStatementType st_type = st->eType;
    // If it is a leaf.
    if (st_type != eIfElse && st_type != eFor) {
      if (rnd_flipcoin(CLOptions::EMI_p_leaf())) del_stms.push_back(st);
      continue;
    }
    // Is a compound statement.
    if (rnd_flipcoin(CLOptions::EMI_p_compound())) {
      del_stms.push_back(st);
      continue;
    }
    // Even if we lift, the block should still be pruned.
    if (st_type == eIfElse) {
      StatementIf *st_if = dynamic_cast<StatementIf *>(st);
      assert(st_if != NULL);
      // 'const' haha yeah right.
      PruneBlock(const_cast<Block *>(st_if->get_true_branch()));
      PruneBlock(const_cast<Block *>(st_if->get_false_branch()));
    } else {
      StatementFor *st_for = dynamic_cast<StatementFor *>(st);
      assert(st_for != NULL);
      PruneBlock(const_cast<Block *>(st_for->get_body()));
    }
    // TODO lift (if lifting for st, need to remove cont/break).
    // Also move local vars.
    // Also adjust prob of p_lift, as this and p_compound are not independent.
  }
  // Remove all the selected statements from the block.
  for (Statement *st : del_stms)
    block->remove_stmt(st);
}

EMIController *EMIController::GetEMIController() {
  if (emi_controller_inst == NULL) emi_controller_inst = CreateEMIController();
  return emi_controller_inst;
}

void EMIController::ReleaseEMIController() {
  delete emi_controller_inst;
  emi_controller_inst = NULL;
}

EMIController *EMIController::CreateEMIController() {
  // TODO this may need to be __const
  return new EMIController(MemoryBuffer::CreateMemoryBuffer(
      MemoryBuffer::kGlobal, "emi_input", &Type::get_simple_type(eInt), NULL,
      1024));
}

void EMIController::AddItemisedEMIInput(MemoryBuffer *item) {
  assert(item->collective == emi_input_.get());
  itemised_emi_input_.push_back(item);
}

void EMIController::PruneEMISections() {
  for (StatementEMI *emi : emi_sections_) emi->Prune();
}

}  // namespace CLSmith
