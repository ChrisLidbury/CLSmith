#include "CLSmith/StatementAtomicResult.h"
#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/ExpressionAtomicAccess.h"
#include "CLSmith/CLStatement.h"

#include "Block.h"
#include "Expression.h"
#include "ExpressionFuncall.h"
#include "Function.h"
#include "Type.h"
#include "Variable.h"

#include <map>

namespace CLSmith {
  
namespace {
static std::map<int, const ExpressionAtomicAccess*>* atomic_blocks = NULL;
}

void StatementAtomicResult::InitResults() {
  atomic_blocks = new std::map<int, const ExpressionAtomicAccess*>();
}
  
void StatementAtomicResult::GenSpecialVals() {
  for (Function* f : get_all_functions()) {
    for (Block* b : f->blocks) {
      if (atomic_blocks->find(b->stm_id) != atomic_blocks->end()) {
        StatementAtomicResult* sar_decl = new StatementAtomicResult(b);
        b->stms.push_back(sar_decl);
        for (Variable* v : b->local_vars) {
          if (v->get_collective() != v || v->type->eType == ePointer)
            continue;
          ArrayVariable* av = dynamic_cast<ArrayVariable*>(v);
          if (av != 0) {
            std::vector<unsigned int> accesses = av->get_sizes();
            std::vector<int> curr_index (accesses.size(), 0);
            for (unsigned int i = 0; i < accesses.size(); i++) {
              accesses[i]--;
              curr_index[i] = accesses[i];
            }
            int index = accesses.size() - 1;
            while (true) {
              while (curr_index[index] >= 0) {
                StatementAtomicResult* sar_res = new StatementAtomicResult(av->itemize(curr_index), b);
                b->stms.push_back(sar_res);
                curr_index[index]--;
              }
              do {
                curr_index[index] = accesses[index];
                index--;
              } while (curr_index[index] == 0);
              if (index < 0) break;
              curr_index[index]--;
              index = curr_index.size() - 1;
            }
          }
          else if (!v->field_vars.empty()) {
            for (Variable* fv : v->field_vars) {
              StatementAtomicResult* sar_res = new StatementAtomicResult(fv, b);
              b->stms.push_back(sar_res);
            }
          }
          else {
            StatementAtomicResult* sar_res = new StatementAtomicResult(v, b);
            b->stms.push_back(sar_res);
          }
        }
        StatementAtomicResult* sar_sv = new StatementAtomicResult((*atomic_blocks->find(b->stm_id)).second, b);
        b->stms.push_back(sar_sv);
      }
    }
  }
}

void StatementAtomicResult::RecordIfID(int id, Expression* expr) {
  ExpressionFuncall* ef = dynamic_cast<ExpressionFuncall*>(expr);
  const ExpressionAtomic* ea = NULL;
  for (const Expression* ef_param : ef->get_invoke()->param_value)
    if ((ea = dynamic_cast<const ExpressionAtomic*>(ef_param)))
      break;
  assert(ea);
  atomic_blocks->insert(std::pair<int, const ExpressionAtomicAccess*>(id, ea->get_access()));
}

void StatementAtomicResult::Output(std::ostream& out, FactMgr* fm, int indent) const {
  output_tab(out, indent);
  switch(result_type_) {
    case kAddVar  : { out << "result += " << var_->to_string() << ";"; break; }
    case kSetSpVal: { 
      assert(access_ != NULL);
      out << "atomic_add(&";
      if (this->access_->is_global()) {
	ExpressionAtomic::GetSVBuffer()->Output(out);
      } else if (this->access_->is_local()) {
	ExpressionAtomic::GetLocalSVBuffer()->Output(out);
      } else assert(0);
      out << "[";
      this->access_->Output(out);
      out << "], result);"; break; }
    case kDecl    : { out << "unsigned int result = 0;"; break; }
    default       : assert(0);
  }
  out << std::endl;
}

void StatementAtomicResult::DefineLocalResultVar(std::ostream& out) {
  out << "unsigned int result = 0";
}

} // namespace CLSmith