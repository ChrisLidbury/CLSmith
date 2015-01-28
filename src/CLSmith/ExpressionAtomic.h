// Expression for hanling atomic operations in OpenCL
// Currently, only generated as if condition
//
// TODO
// * make separate StatementAtomic handling entire block generation

#ifndef _CLSMITH_EXPRESSIONATOMIC_H_
#define _CLSMITH_EXPRESSIONATOMIC_H_

#include "CLSmith/CLExpression.h"
#include "CLSmith/Globals.h"
#include "CLSmith/MemoryBuffer.h"

#include <set>
#include <vector>

#include "ArrayVariable.h"
#include "CGContext.h"
#include "Expression.h"
#include "Type.h"

namespace CLSmith {

/* Class representing the indexing for the atomic array variables */

class ExpressionAtomicAccess : public CLExpression {
 public:
  enum AtomicMemAccess {
    kGlobal = 0,
    kLocal
  };
   
  ExpressionAtomicAccess (const int constant, const AtomicMemAccess mem) : CLExpression(kAtomic),
    constant_(constant), mem_(mem), type_(Type::get_simple_type(eInt)) {}
    
  // TODO
  bool is_global(void) const;
  bool is_local(void) const;
    
  // TODO
  int get_counter(void) const {return this->constant_;};
  
  // Pure virtual methods from Expression
  ExpressionAtomicAccess *clone() const { return NULL; };
  const Type &get_type() const { return type_; };
  CVQualifiers get_qualifiers() const { return CVQualifiers(true, false); }
  void get_eval_to_subexps(std::vector<const Expression*>& subs) const {}
  void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const {}
  unsigned get_complexity() const { return 1; }

  void Output(std::ostream& out) const;
  
 private:
  const int constant_;
  const AtomicMemAccess mem_;
  
//   Expression::AtomicMemory mem_;
  const Type& type_;
  
  DISALLOW_COPY_AND_ASSIGN(ExpressionAtomicAccess);
};
  
class ExpressionAtomic : public CLExpression {
 public:
  // Atomic expression type
  enum AtomicExprType {
    kAdd = 0,
    kSub,
    kInc,
    kDec,
    kXchg,
    kCmpxchg,
    kMin,
    kMax, 
    kAnd,
    kOr,
    kXor
  };
    
  // Three constructors for the three possible operations:
  // * inc and dec have one parameter;
  // * cmpxchg has three parameters;
  // * other operations have two parameters.
  // For simplicity, all the parameters are considered int, coming from a
  // __global volatile array passed as a parameter to the kernel.
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var, const ExpressionAtomicAccess* eaa) : 
    CLExpression(kAtomic), global_var_(global_var), access_(eaa), 
    atomic_type_(type), type_(Type::get_simple_type(eInt)) {}
  
  ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var, const ExpressionAtomicAccess* eaa, int val) : 
    CLExpression(kAtomic), global_var_(global_var), access_(eaa),
    val_(val), atomic_type_(type), type_(Type::get_simple_type(eInt)) {}
    
    // Unused; only for cmpxchng
//   ExpressionAtomic (AtomicExprType type, ArrayVariable* global_var, int val, int cmp) : 
//     CLExpression(kAtomic), global_var_(global_var),
//     val_(val), cmp_(cmp), atomic_type_(type), type_(Type::get_simple_type(eInt)) {}
    

  // TODO
  const ExpressionAtomicAccess* get_access(void) const {return this->access_;} ;
    
  // Generates a random atomic expression; all expressions have equal chance of 
  // being generated.
  static ExpressionAtomic *make_random(CGContext &cg_context, const Type *type);
  
  // Generates a random expression of the form "<atomic-expr> == <constant>" to
  // be used as the condition for an if block.
  static Expression *make_condition(CGContext &cg_context, const Type *type);
  
  // This represents the __global volatile array parameter from the kernel;
  // the underlying object is a singleton MemoryBuffer.
  static MemoryBuffer* GetGlobalBuffer();
  static MemoryBuffer* GetSVBuffer();
  static MemoryBuffer* GetLocalBuffer();
  static MemoryBuffer* GetLocalSVBuffer();
  
  // Initialize various parameters related to atomic blocks
  static void InitAtomics(void);
  
  // Return the number of maximum atomic blocks for the current program
  static int get_atomic_blocks_no(void);
  
  // Add required variables to the global buffer
  static void AddVarsToGlobals(Globals* globals);
  
  // Since all the p arguments of the atomic expressions come from the global
  // array, it must be held in the struct, for global access. Here we save all
  // array accesses, which we will add after the program generation to the 
  // global struct; this is required as the global struct should not get
  // generated until after the entire program is complete.
  static std::vector<MemoryBuffer*>* GetGlobalMems();
  static std::vector<MemoryBuffer*>* GetSVMems();
  static std::vector<MemoryBuffer*>* GetLocalMems();
  static std::vector<MemoryBuffer*>* GetLocalSVMems();
  static bool HasSVMems();
  static bool HasLocalSVMems();
  
  // TODO
  static std::vector<Variable *>* GetBlockVars(Block* b);
  
  // Called in Block::make_random(); whenever we create a block within an atomic
  // context, we add all the local variables (except the ArrayVariables that are
  // part of a collective) into the block_vars vector
  static void InsertBlockVars(std::vector<Variable*> local_vars, std::vector<Variable*>* blk_vars);
  
  // TODO
  static void AddBlockVar(Variable* v);
  static void AddBlockVar(const Variable* v);
  
  // TODO
  static void GenBlockVars(Block* curr);
  
  // TODO
  static void RemoveBlockVars(Block* b);
  static void DelBlockVars();
  
  // Pure virtual methods from Expression
  ExpressionAtomic *clone() const { return NULL; };
  const Type &get_type() const { return type_; };
  CVQualifiers get_qualifiers() const { return CVQualifiers(true, false); }
  void get_eval_to_subexps(std::vector<const Expression*>& subs) const {}
  void get_referenced_ptrs(std::vector<const Variable*>& ptrs) const {}
  unsigned get_complexity() const { return 1; }

  void Output(std::ostream& out) const;
  
  // Prints out the required code to hash the special values in the results
  static void OutputHashing(std::ostream& out);
    
 private:
  // Represents the p parameter of the expression; it is an array access into
  // the global array parameter
  ArrayVariable* global_var_;
  const ExpressionAtomicAccess* access_;
  
  // The two additional parameters required by certain operations
  // TODO consider making Variable
  int val_;
  int cmp_;
  
  // The type of the expression, one for CLSmith and the other for CSmith
  AtomicExprType atomic_type_;
  const Type& type_;
  
  // Helper function returning the operation name for the Output() function
  std::string GetOpName() const;
  
  // Creates a new vector to hold variables created in the current block;
  // must only be called from make_condition()
  static void MakeBlockVars(Block* parent);
  
  static void PrintBlockVars();
  
  
  DISALLOW_COPY_AND_ASSIGN(ExpressionAtomic);
};

} // namespace CLSmith

#endif // _CLSMITH_EXPRESSIONATOMIC_H_
