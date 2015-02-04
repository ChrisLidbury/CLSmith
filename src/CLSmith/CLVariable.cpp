/*TODO Work in progress 
    * rewrite to see if we can use FunctionWalker */

#include "CLVariable.h"

#include "Block.h"
#include "Expression.h"
#include "ExpressionAssign.h"
#include "ExpressionComma.h"
#include "ExpressionFuncall.h"
#include "ExpressionVariable.h"
#include "CLSmith/ExpressionVector.h"
#include "CLSmith/Walker.h"
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
#include <algorithm>

namespace CLSmith {

void CLVariable::ParseUnusedVars2() {	
  Walker::FunctionWalker* wlker = new Walker::FunctionWalker(GetFirstFunction());
  while (wlker->Advance()) {
    switch(wlker->GetCurrentStatement()->eType) {
      case eAssign: std::cout << "ass" << std::endl;
      case eReturn: std::cout << "ret" << std::endl;
      default: std::cout << "ceva" << std::endl;
    }
  }
}

void CLVariable::ParseUnusedVars() {
  // consider changing to set
  std::vector<Function*> funcs = get_all_functions();
  std::set<const Variable*> accesses;

  for (Function * f : funcs) {
    for (Block * b : f->blocks) {
      Helpers::RecordVarInitialisation(&accesses, b);
      for (Statement * s : b->stms) {
        switch(s->eType) {
          case eAssign: 
            { StatementAssign* as = dynamic_cast<StatementAssign*>(s);
              const Variable* lhsv = as->get_lhs()->get_var()->get_collective();
              std::set<const Variable*>* rhsv = 
                Helpers::GetVarsFromExpr(as->get_expr());
              Helpers::RecordVar(&accesses, lhsv);
              Helpers::RecordMultipleVars(&accesses, rhsv);
              break; }
          case eReturn: 
            { StatementReturn* rs = dynamic_cast<StatementReturn*>(s);
              const Variable* rsv = rs->get_var()->get_var()->get_collective();
              Helpers::RecordVar(&accesses, rsv);
              break; }
          case eFor: 
            { StatementFor* sf = dynamic_cast<StatementFor*>(s);
              const Variable* initlhsv = 
                sf->get_init()->get_lhs()->get_var()->get_collective();
              const Variable* incrlhsv = 
                sf->get_init()->get_lhs()->get_var()->get_collective();
              std::set<const Variable*>* initrhsv = 
                Helpers::GetVarsFromExpr(sf->get_init()->get_expr());
              std::set<const Variable*>* testv = 
                Helpers::GetVarsFromExpr(sf->get_test());
              std::set<const Variable*>* incrrhsv = 
                Helpers::GetVarsFromExpr(sf->get_incr()->get_expr());
              Helpers::RecordVar(&accesses, initlhsv);
              Helpers::RecordVar(&accesses, incrlhsv);
              Helpers::RecordMultipleVars(&accesses, initrhsv);
              Helpers::RecordMultipleVars(&accesses, testv);
              Helpers::RecordMultipleVars(&accesses, incrrhsv);
              break; }
          case eIfElse: 
            { StatementIf* si = dynamic_cast<StatementIf*>(s);
              std::set<const Variable*>* testv = 
                Helpers::GetVarsFromExpr(si->get_test());
              Helpers::RecordMultipleVars(&accesses, testv);
              break; }
          case eInvoke: 
            { StatementExpr* se = dynamic_cast<StatementExpr*>(s);
              std::set<const Variable*>* exprv = 
                Helpers::GetVarsFromExpr(se->get_call());
              Helpers::RecordMultipleVars(&accesses, exprv);
              break; }
          case eContinue: 
            { StatementContinue* sc = dynamic_cast<StatementContinue*>(s);
              std::set<const Variable*>* testv = 
                Helpers::GetVarsFromExpr(&sc->test);
              Helpers::RecordMultipleVars(&accesses, testv);
              break; }
          case eBreak: 
            { StatementBreak* sb = dynamic_cast<StatementBreak*>(s);
              std::set<const Variable*>* testv = 
                Helpers::GetVarsFromExpr(&sb->test);
              Helpers::RecordMultipleVars(&accesses, testv);
              break; }
          case eGoto: 
            { StatementGoto* sg = dynamic_cast<StatementGoto*>(s);
              std::set<const Variable*>* testv = 
                Helpers::GetVarsFromExpr(&sg->test);
              Helpers::RecordMultipleVars(&accesses, testv);
              break; }
          case eArrayOp: 
            { StatementArrayOp* sao = dynamic_cast<StatementArrayOp*>(s);
              const Variable* arrv = sao->array_var->get_collective();
              std::set<const Variable*> ctrlv 
                (sao->ctrl_vars.begin(), sao->ctrl_vars.end());
              std::set<const Variable*>* initv = 
                Helpers::GetVarsFromExpr(sao->init_value);
              Helpers::RecordVar(&accesses, arrv);
              Helpers::RecordMultipleVars(&accesses, &ctrlv);
              Helpers::RecordMultipleVars(&accesses, initv);
              break; }
          case eCLStatement: 
          case eBlock: 
            break;
          default:
            assert(0);
        }
      }
    }
  }
  
  for (Function* f : funcs) {
    for (Block* b : f->blocks) {
      for (Variable* v : b->local_vars) {
        if (std::find(accesses.begin(), accesses.end(), v) == accesses.end()) {
          b->local_vars.erase(std::remove(b->local_vars.begin(), b->local_vars.end(), v), b->local_vars.end());
        }
      }
    }
  }
}

std::set<const Variable*>* CLVariable::Helpers::GetVarsFromExpr(const Expression* expr) {
  std::set<const Variable*>* expr_vars = new std::set<const Variable*>();
  switch (expr->term_type) {
    case eVariable: 
      { const ExpressionVariable* ev = 
          dynamic_cast<const ExpressionVariable*>(expr);
        expr_vars->insert(ev->get_var()->get_collective()); break; }
    case eFunction: 
      { const ExpressionFuncall* efun = 
          dynamic_cast<const ExpressionFuncall*>(expr); 
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
        std::set<const Variable*>* inner_expr_lhs = 
          GetVarsFromExpr(ec->get_lhs());
        std::set<const Variable*>* inner_expr_rhs = 
          GetVarsFromExpr(ec->get_rhs());
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

void CLVariable::Helpers::RecordVarInitialisation(
std::set<const Variable*>* accesses, Block* b) {
  for (const Variable* v : b->local_vars) {
    std::set<const Variable*>* init_vars = GetVarsFromExpr(v->init);
    RecordMultipleVars(accesses, init_vars);
  }
}

void CLVariable::Helpers::RecordVar(
std::set<const Variable*>* accesses, const Variable* var) {
  // ignore global and parameter variables
  if (!var->to_string().compare(0, 2, "g_") 
   || !var->to_string().compare(0, 2, "p_")) {
    return;
  }
  accesses->insert(var);
}

void CLVariable::Helpers::RecordMultipleVars(
std::set<const Variable*>* accesses, 
std::set<const Variable*>* vars) {
  for (const Variable* v : *vars) {
    RecordVar(accesses, v);
  }
}

}
