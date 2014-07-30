// A very limited representation of a local/global memory buffer in OpenCL.
// Instead of generalising the memory spaces for any type, we limit them to
// buffers.
// We could instead extend CVQualifiers, but this still leaves a few problems.
// TODO: Make better :> maybe extending CVQualifiers is better.

#ifndef _CLSMITH_MEMORYBUFFER_H_
#define _CLSMITH_MEMORYBUFFER_H_

#include <fstream>
#include <string>
#include <vector>

#include "ArrayVariable.h"
#include "CommonMacros.h"

class CVQualifiers;
class Expression;
class Type;

namespace CLSmith {

// An OpenCL memory buffer, which is pretty much just an array.
// Extend the ArrayVariable, although there are some differences and limitations
// on a memory buffer. Most of these differences are syntactic.
// TODO: Proper interaction with ArrayVariable.
// TODO: Use init, allow init other than all constant.
class MemoryBuffer : public ArrayVariable {
 public:
  enum MemorySpace {
    kGlobal = 0,
    kLocal,
    kPrivate,
    kConst
  };

  MemoryBuffer(MemorySpace memory_space, const std::string &name,
      const Type *type, const Expression* init, const CVQualifiers *qfer,
      unsigned size)
      : ArrayVariable(NULL, name, type, init, qfer, {size}, NULL, false),
      memory_space_(memory_space) {
  }
  MemoryBuffer(MemoryBuffer&& other) = default;
  MemoryBuffer& operator=(MemoryBuffer&& other) = default;
  MemoryBuffer(const MemoryBuffer& other) = default;
  MemoryBuffer& operator=(const MemoryBuffer& other) = default;
  virtual ~MemoryBuffer() {}

  // Static factory, mostly just forwards it to the constructor. Does not create
  // a random array, must be set by the user.
  static MemoryBuffer *CreateMemoryBuffer(MemorySpace memory_space,
      const std::string &name, const Type *type, const Expression* init,
      unsigned size);
  
  // Takes a list of indices and creates a new memory buffer that accesses
  // the elements at those indices; the number of indices in the list must
  // be the same as the number of dimensions of the array
  MemoryBuffer* itemize(const vector<int>& const_indices) const;

  // Print the memory space qualifier.
  static void OutputMemorySpace(std::ostream& out, MemorySpace memory_space);
  // Output with an access to the item blonging to the thread (if any).
  void OutputWithOwnedItem(std::ostream& out) const;

  // Override outputs from ArrayVariable that next to print the memory space or
  // only access a specific part of the buffer.
  void hash(std::ostream& out) const;
  void OutputDef(std::ostream& out, int indent) const;
  void output_qualified_type(std::ostream& out) const;

  // Alias must be declared with a * instead of a [] for some reason.
  void OutputAliasDecl(std::ostream& out) const;

  MemorySpace GetMemorySpace() { return memory_space_; }
 private:
  MemorySpace memory_space_;
};

}  // namespace CLSmith

#endif  // _CLSMITH_MEMORYBUFFER_H_
