#include "CLSmith/Globals.h"

#include <ostream>

#include "Type.h"
#include "Variable.h"
#include "VariableSelector.h"

class CVQualifiers;

namespace CLSmith {

void Globals::OutputStructDefinition(std::ostream& out) {
  std::vector<const Type *> types;
  std::vector<CVQualifiers> qfers;
  std::vector<int> bitfield_len;
  for (Variable *var : global_vars_) {
    types.push_back(var->type);
    //qfers.push_back(CVQualifiers(true, true));
    qfers.push_back(var->qfer);
    bitfield_len.push_back(-1);
  }
  Type global_struct(types, true, false, qfers, bitfield_len);
  //global_struct.Output(out);
  OutputStructUnion(&global_struct, out);
}

void Globals::OutputStructInit(std::ostream& out) {

}

Globals Globals::CreateGlobals() {
  return Globals(*VariableSelector::GetGlobalVariables());
}

}  // namespace CLSmith
