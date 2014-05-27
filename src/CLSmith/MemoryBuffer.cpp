#include "CLSmith/MemoryBuffer.h"

#include <iostream>
#include <string>
#include <vector>

#include "ArrayVariable.h"
#include "CVQualifiers.h"
#include "util.h"
#include "Variable.h"

namespace CLSmith {

MemoryBuffer *MemoryBuffer::CreateMemoryBuffer(MemorySpace memory_space,
    const std::string &name, const Type *type, const Expression* init,
    unsigned size) {
  bool cnst = memory_space == kConst;
  bool vol = false;
  CVQualifiers qfer(std::vector<bool>({cnst}), std::vector<bool>({vol}));
  return new MemoryBuffer(memory_space, name, type, init, &qfer, size);
}

void MemoryBuffer::OutputMemorySpace(
    std::ostream& out, MemorySpace memory_space) {
  switch (memory_space) {
    case kGlobal:  out << "__global";  break;
    case kLocal:   out << "__local";   break;
    case kPrivate: out << "__private"; break;
    case kConst:   out << "__const";   break;
  }
}

void MemoryBuffer::OutputWithOwnedItem(std::ostream& out) const {
  Output(out);
  out << (memory_space_ == kGlobal ? "[get_global_id(0)]" :
      memory_space_ == kLocal ? "[get_local_id(0)]" : "");
}

void MemoryBuffer::hash(std::ostream& out) const {
  if (memory_space_ == kConst) return;

  if (memory_space_ == kPrivate) {
    ArrayVariable::hash(out);
    return;
  }

  output_tab(out, 1);
  out << "transparent_crc(";
  OutputWithOwnedItem(out);
  out << ", \"";
  OutputWithOwnedItem(out);
  out << "\", print_hash_value);" << std::endl;
}

void MemoryBuffer::OutputDef(std::ostream& out, int indent) const {
  if (memory_space_ == kPrivate || memory_space_ == kConst) {
    ArrayVariable::OutputDef(out, indent);
    return;
  }

  output_tab(out, indent);
  OutputDecl(out);
  out << ';' << std::endl;
  output_tab(out, indent);
  OutputWithOwnedItem(out);
  out << " = ";
  init->Output(out);
  out << ";" << std::endl;
}

void MemoryBuffer::output_qualified_type(std::ostream& out) const {
  OutputMemorySpace(out, memory_space_);
  out << " ";
  Variable::output_qualified_type(out);
}

void MemoryBuffer::OutputAliasDecl(std::ostream& out) const {
  output_qualified_type(out);
  out << "*";
  Output(out);
}

}  // namespace CLSmith
