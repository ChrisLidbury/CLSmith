#ifndef _CLSMITH_CLVARIABLE_H_
#define _CLSMITH_CLVARIABLE_H_

#include "Variable.h"
#include "Expression.h"
#include "FunctionInvocation.h"

#include <set>

namespace CLSmith {
namespace CLVariable {
  
void ParseUnusedVars(void);

namespace Helpers {
  
std::set<const Variable*>* GetVarsFromExpr(const Expression* expr);
void RecordVar(std::map<int, std::set<const Variable*>*>* accesses, const Variable* var, Block* b);
void RecordMultipleVars(std::map<int, std::set<const Variable*>*>* accesses, std::set<const Variable*>* vars, Block* b);
void PrintMap(std::map<int, std::set<const Variable*>*>* accesses);

} // namespace Helpers
} // namespace CLVariable
} // namespace CLSmith

#endif  //  _CLSMITH_CLVARIABLE_H_