#ifndef _CLSMITH_CLVARIABLE_H_
#define _CLSMITH_CLVARIABLE_H_

#include "Variable.h"
#include "Expression.h"
#include "FunctionInvocation.h"

#include <set>

namespace CLSmith {
namespace CLVariable {
  
/* To be called once the whole random program is generated;
 * It traverses it and identifies all variables that are actually used in
 * expressions; it then removes any variables that are declared, but not 
 * referenced anywhere
 * TODO maybe leave in some variables or add a chance of clearing a variable
 ***/
void ParseUnusedVars(void);

namespace Helpers {
/* This contains helper functions for the methods in the main namespace;
 * they should not be called from anywhere else (consider them private) 
 ***/

/* Given an Expression, returns all variables referenced in it
 ***/
std::set<const Variable*>* GetVarsFromExpr(const Expression* expr);

/* Stores var inside accesses, where var is referenced in block b;
 * This performs some additional checks regarding var, such as its scope 
 * (currently, we ignore global and parameter variables; might revise the 
 * decision about globals); the int key of the map refers to the block id
 * (stm_id) where the given variable is declared, not where it is referenced
 ***/
void RecordVar(std::map<int, std::set<const Variable*>*>* accesses, const Variable* var, Block* b);

/* Records all the variables in vars by calling RecordVar() multiple times
 ***/
void RecordMultipleVars(std::map<int, std::set<const Variable*>*>* accesses, std::set<const Variable*>* vars, Block* b);

/* Method for printing the contents of the map that retains variable reference
 * information
 ***/
void PrintMap(std::map<int, std::set<const Variable*>*>* accesses);

} // namespace Helpers
} // namespace CLVariable
} // namespace CLSmith

#endif  //  _CLSMITH_CLVARIABLE_H_