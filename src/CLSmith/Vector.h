// Vectors!

#ifndef _CLSMITH_VECTOR_H_
#define _CLSMITH_VECTOR_H_

#include <ostream>
#include <string>
#include <vector>

#include "ArrayVariable.h"
#include "CGContext.h"

class Block;
class CVQualifiers;
class Expression;
class Type;
class Variable;

namespace CLSmith {

// Handles OpenCL's vector type.
// Treated mostly the same as an array during generation. Only the output and
// size need special handling.
class Vector : public ArrayVariable {
 public:
  // Basic constructor with parameters required by ArrayVariable.
  // Will only accept 1-dimensional sizes.
  Vector(Block* blk, const std::string& name, const Type *type,
      const Expression *init, const CVQualifiers *qfer, int size,
      const Variable *isFieldVarOf)
      : ArrayVariable(blk, name, type, init, qfer, {size}, isFieldVarOf, true) {
  }

  // Copy constructor required, as ArrayVariable frequently uses it during
  // itemisation.
  Vector(const Vector& other) = default;
  Vector& operator=(const Vector& other) = default;
  // No move constructors in ArrayVariable.
  Vector(Vector&& other) = delete;
  Vector& operator=(Vector&& other) = delete;
  virtual ~Vector() {}

  // Factory for producing a vector variable.
  // Parameters are mostly just forwarded on to the ArrayVariable constructor.
  static Vector *CreateVectorVariable(const CGContext& cg_context, Block *blk,
      const std::string& name, const Type *type, const Expression *init,
      const CVQualifiers *qfer, const Variable *isFieldVarOf);

  // Itemise methods for using a specific entry in the vector. All accesses must
  // be a constant index, so the last two methods will randomise the access.
  Vector *itemize(void) const;
  Vector *itemize(const std::vector<int>& const_indices) const;
  Vector *itemize(
      const std::vector<const Variable*>& indices, Block* blk) const {
    return itemize();
  }
  Vector *itemize(
      const std::vector<const Expression*>& indices, Block* blk) const {
    return itemize();
  }
  // Itemise for accessing multiple components at once.
  virtual Vector *itemize_simd(void) const;
  virtual Vector *itemize_simd(int access_count) const;
  virtual Vector *itemize_simd(const std::vector<int>& const_indices) const;

  // Methods that control how the array is printed.
  // In all cases, there is only one index, which is printed as a character
  // after a dot:
  //  'vec[2]' -> 'vec.z' or 'vec.s2'
  //  '{vec[3], vec[1]}' -> 'vec.wy' or 'vec.s31'
  void Output(std::ostream& out) const;
  void OutputDef(std::ostream& out, int indent) const;
  void OutputDecl(std::ostream& out) const; 
  void hash(std::ostream& out) const;

 private:
  // Returns the character corresponding to the component that is accessed.
  // If the size of the vector is greater than 4, the access character is a hex
  // digit, otherwise, it is one of xyzw.
  char GetComponentChar(int index) const;

  // Keeps track of multiple accesses in a single itemisation.
  // e.g. 'vec.wy' will be {3, 1}
  std::vector<int> comp_access_;
};

}  // namespace CLSmith

#endif  // CLSMITH_VECTOR_H_
