#include "CLVariable.h"

#include "Block.h"
#include "Expression.h"
#include "ExpressionAssign.h"
#include "ExpressionComma.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "CLSmith/ExpressionVector.h"
#include "Function.h"
#include "FunctionInvocation.h"
#include "FunctionInvocationUnary.h"
#include "Statement.h"
#include "StatementAssign.h"
#include "StatementArrayOp.h"
#include "StatementBreak.h"
#include "StatementContinue.h"
#include "StatementExpr.h"
#include "StatementFor.h"
#include "StatementGoto.h"
#include "StatementIf.h"
#include "StatementReturn.h"
#include "Variable.h"

#include <set>
#include <vector>
#include <map>

namespace CLSmith {

void CLVariable::ParseUnusedVars() {
  // consider changing to set
  std::vector<Function*> funcs = get_all_functions();
  std::map<int, std::set<const Variable*>*> accesses;

  for (Function * f : funcs) {
    for (Block * b : f->blocks) {
      for (Statement * s : b->stms) {
        switch(s->eType) {
          case eAssign: 
            { StatementAssign* as = dynamic_cast<StatementAssign*>(s);
              const Variable* lhsv = as->get_lhs()->get_var()->get_collective();
              std::set<const Variable*>* rhsv = Helpers::GetVarsFromExpr(as->get_expr());
              Helpers::RecordVar(&accesses, lhsv, b);
              Helpers::RecordMultipleVars(&accesses, rhsv, b);
              break; }
          case eReturn: 
            { StatementReturn* rs = dynamic_cast<StatementReturn*>(s);
              const Variable* rsv = rs->get_var()->get_var()->get_collective();
              Helpers::RecordVar(&accesses, rsv, b);
              break; }
          case eFor: 
            { StatementFor* sf = dynamic_cast<StatementFor*>(s);
              const Variable* initlhsv = sf->get_init()->get_lhs()->get_var()->get_collective();
              const Variable* incrlhsv = sf->get_init()->get_lhs()->get_var()->get_collective();
              std::set<const Variable*>* initrhsv = Helpers::GetVarsFromExpr(sf->get_init()->get_expr());
              std::set<const Variable*>* testv = Helpers::GetVarsFromExpr(sf->get_test());
              std::set<const Variable*>* incrrhsv = Helpers::GetVarsFromExpr(sf->get_incr()->get_expr());
              Helpers::RecordVar(&accesses, initlhsv, b);
              Helpers::RecordVar(&accesses, incrlhsv, b);
              Helpers::RecordMultipleVars(&accesses, initrhsv, b);
              Helpers::RecordMultipleVars(&accesses, testv, b);
              Helpers::RecordMultipleVars(&accesses, incrrhsv, b);
              break; }
          case eIfElse: 
            { StatementIf* si = dynamic_cast<StatementIf*>(s);
              std::set<const Variable*>* testv = Helpers::GetVarsFromExpr(si->get_test());
              Helpers::RecordMultipleVars(&accesses, testv, b);
              break; }
          case eInvoke: 
            { StatementExpr* se = dynamic_cast<StatementExpr*>(s);
              std::set<const Variable*>* exprv = Helpers::GetVarsFromExpr(se->get_call());
              Helpers::RecordMultipleVars(&accesses, exprv, b);
              break; }
          case eContinue: 
            { StatementContinue* sc = dynamic_cast<StatementContinue*>(s);
              std::set<const Variable*>* testv = Helpers::GetVarsFromExpr(&sc->test);
              Helpers::RecordMultipleVars(&accesses, testv, b);
              break; }
          case eBreak: 
            { StatementBreak* sb = dynamic_cast<StatementBreak*>(s);
              std::set<const Variable*>* testv = Helpers::GetVarsFromExpr(&sb->test);
              Helpers::RecordMultipleVars(&accesses, testv, b);
              break; }
          case eGoto: 
            { StatementGoto* sg = dynamic_cast<StatementGoto*>(s);
//               sg->Output(std::cout, get_fact_mgr_for_func(f));
              std::set<const Variable*>* testv = Helpers::GetVarsFromExpr(&sg->test);
              Helpers::RecordMultipleVars(&accesses, testv, b);
              break; }
          case eCLStatement: { /*std::cout << "cls" << std::endl;*/ break; }
          case eArrayOp: 
            { StatementArrayOp* sao = dynamic_cast<StatementArrayOp*>(s);
//               sao->Output(std::cout, get_fact_mgr_for_func(f)); 
              const Variable* arrv = sao->array_var->get_collective();
              std::set<const Variable*> ctrlv (sao->ctrl_vars.begin(), sao->ctrl_vars.end());
              std::set<const Variable*>* initv = Helpers::GetVarsFromExpr(sao->init_value);
              Helpers::RecordVar(&accesses, arrv, b);
              Helpers::RecordMultipleVars(&accesses, &ctrlv, b);
              Helpers::RecordMultipleVars(&accesses, initv, b);
              break; }
          case eBlock: 
            break;
          default:
            assert(0);
        }
      }
    }
  }
  
//   Helpers::PrintMap(&accesses);
}

std::set<const Variable*>* CLVariable::Helpers::GetVarsFromExpr(const Expression* expr) {
  std::set<const Variable*>* expr_vars = new std::set<const Variable*>();
  switch (expr->term_type) {
    case eVariable: 
      { const ExpressionVariable* ev = dynamic_cast<const ExpressionVariable*>(expr);
        expr_vars->insert(ev->get_var()->get_collective()); break; }
    case eFunction: 
      { const ExpressionFuncall* efun = dynamic_cast<const ExpressionFuncall*>(expr); 
        for (const Expression* e : efun->get_invoke()->param_value) {
          std::set<const Variable*>* inner_expr = GetVarsFromExpr(e);
          expr_vars->insert(inner_expr->begin(), inner_expr->end());
        }
        break; }
    case eAssignment: 
      { const ExpressionAssign* ea = dynamic_cast<const ExpressionAssign*>(expr);
        expr_vars->insert(ea->get_lhs()->get_var()->get_collective());
        std::set<const Variable*>* inner_expr = GetVarsFromExpr(ea->get_rhs());
        expr_vars->insert(inner_expr->begin(), inner_expr->end());
        break; }
    case eCLExpression: 
      { const CLExpression* cle = dynamic_cast<const CLExpression*>(expr);
        if (cle->GetCLExpressionType() != CLExpression::kVector)
          break;
        const ExpressionVector* ev = dynamic_cast<const ExpressionVector*>(expr);
        for (const std::unique_ptr<const Expression>& e : ev->GetExpressions()) {
          const Expression* e_ptr = e.get();
          std::set<const Variable*>* inner_expr = GetVarsFromExpr(e_ptr);
          expr_vars->insert(inner_expr->begin(), inner_expr->end());
        }
        break; }
    case eCommaExpr: 
      { const ExpressionComma* ec = dynamic_cast<const ExpressionComma*>(expr);
        std::set<const Variable*>* inner_expr_lhs = GetVarsFromExpr(ec->get_lhs());
        std::set<const Variable*>* inner_expr_rhs = GetVarsFromExpr(ec->get_rhs());
        expr_vars->insert(inner_expr_lhs->begin(), inner_expr_lhs->end());
        expr_vars->insert(inner_expr_rhs->begin(), inner_expr_rhs->end());
        break;
      }
    case eConstant:
    case eLhs:  // NOTE seems to be very particular case for something 
      break; 
    default:
      assert(0);
  }
  return expr_vars;
}

void CLVariable::Helpers::RecordVar(std::map<int, std::set<const Variable*>*>* accesses, const Variable* var, Block* b) {
  // ignore global and parameter variables
  if (!var->to_string().compare(0, 2, "g_") || !var->to_string().compare(0, 2, "p_")) {
    return;
  }
  while (find(b->local_vars.begin(), b->local_vars.end(), var) == b->local_vars.end())
    b = b->parent;
  assert (b != NULL);
  if (accesses->find(b->stm_id) != accesses->end()) {
    std::set<const Variable*>* old_vec = accesses->at(b->stm_id);
    old_vec->insert(var);
  }
  else {
    std::set<const Variable*>* new_vec = new std::set<const Variable*> ();
    new_vec->insert(var);
    accesses->insert(std::pair<int, std::set<const Variable*>*>(b->stm_id, new_vec));
  }
}

void CLVariable::Helpers::RecordMultipleVars(std::map<int, std::set<const Variable*>*>* accesses, std::set<const Variable*>* vars, Block* b) {
  for (const Variable* v : *vars) {
    RecordVar(accesses, v, b);
  }
}

void CLVariable::Helpers::PrintMap(std::map<int, std::set<const Variable*>*>* accesses) {
  for (std::map<int, std::set<const Variable*>*>::iterator it = accesses->begin(); it != accesses->end(); it++) {
    std::cout << (*it).first << " --- " << std::endl;
    for (const Variable* v : *((*it).second))
      std::cout << v->to_string() << std::endl;
  }
}

}