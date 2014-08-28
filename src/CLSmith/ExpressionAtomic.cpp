#include "CLSmith/ExpressionAtomic.h"
#include "CLSmith/Globals.h"

#include <algorithm>
#include <vector>

#include "ArrayVariable.h"
#include "Block.h"
#include "CGContext.h"
#include "Constant.h"
#include "Expression.h"
#include "ExpressionFuncall.h"
#include "FunctionInvocationBinary.h"
#include "random.h"
#include "VariableSelector.h"


namespace CLSmith {
namespace {
// Corresponds to the global array parameter passed to the kernel
static MemoryBuffer* global_buf = NULL;

// Holds the global array parameter reference and all references to it;
// these need to be added to the global struct when it is created.
// (in CLProgramGenerator::goGenerator())
static std::vector<MemoryBuffer*>* global_mems = NULL;

// To be used during code generation; holds the variables generated in an 
// atomic block and uses them to compute a special value used for checking
static std::vector<Variable *>* block_vars = NULL;
} // namespace
  
// TODO make if from switch (+ other functions too)
ExpressionAtomic* ExpressionAtomic::make_random(CGContext &cg_context, const Type *type) {
  assert(type->eType == eSimple && type->simple_type == eInt);
  std::vector<int> rand_index (1, rnd_upto(GetGlobalBuffer()->get_size()));
  MemoryBuffer* itemized = GetGlobalBuffer()->itemize(rand_index);
  GetGlobalMems()->push_back(itemized);
  AtomicExprType atomic_type = rnd_flipcoin(33) ? kInc : (AtomicExprType) rnd_upto(kXor);
  switch(atomic_type) {
    case kDec:
    case kInc:
      return new ExpressionAtomic(atomic_type, itemized);
    case kAdd:
    case kSub:
    case kXchg:
    case kMin:
    case kMax:
    case kAnd:
    case kOr:
    case kXor: 
      return new ExpressionAtomic(atomic_type, itemized, rnd_upto(100));
    case kCmpxchg:
      return new ExpressionAtomic(atomic_type, itemized, rnd_upto(100), 0);
    default: assert(0); // should never get here.
  }
}

Expression *ExpressionAtomic::make_condition(CGContext &cg_context, const Type *type) {
  assert(type->eType == eSimple && type->simple_type == eInt);
  ExpressionAtomic *eAtomic = make_random(cg_context, type);
  FunctionInvocationBinary *fi = new FunctionInvocationBinary(eCmpEq, eAtomic,
                                  Constant::make_int(0), NULL);
  MakeBlockVars();
  return new ExpressionFuncall(*fi);
}

MemoryBuffer* ExpressionAtomic::GetGlobalBuffer() {
  if (global_buf == NULL)  {
    global_buf = MemoryBuffer::CreateMemoryBuffer(MemoryBuffer::kGlobal,
      "input", &Type::get_simple_type(eInt), NULL, 1024);
    GetGlobalMems()->push_back(global_buf);
  }
  return global_buf;
}

std::vector<MemoryBuffer*>* ExpressionAtomic::GetGlobalMems() {
  if (global_mems == NULL)
    global_mems = new std::vector<MemoryBuffer*>;
  return global_mems;
}

void ExpressionAtomic::MakeBlockVars() {
  if (block_vars != NULL) {
    delete(block_vars);
  }
  block_vars = new std::vector<Variable*>();
}

std::vector<Variable*>* ExpressionAtomic::GetBlockVars() {
  // TODO may need check for non-visible variables
  assert(block_vars != NULL); // MakeBlockVars() must be called before
  return block_vars;
}

void ExpressionAtomic::InsertBlockVars(std::vector<Variable*> local_vars) {
  std::vector<Variable*>* blk_vars = GetBlockVars();
  for (Variable* v : local_vars) {
//     std::cout << "Inserting " << v->to_string() << std::endl;
    ArrayVariable * av = dynamic_cast<ArrayVariable*>(v);
    if (av != NULL && (av->get_collective() != av)) {
      blk_vars->push_back(const_cast<Variable*>(av->get_collective()));
      continue;
    }
    blk_vars->push_back(v);
  }
}

void ExpressionAtomic::AddBlockVar(Variable *v) {
  std::vector<Variable*>* blk_vars = GetBlockVars();
  ArrayVariable * av = dynamic_cast<ArrayVariable*>(v);
//   std::cout << "Adding " << v->to_string() << std::endl;
  if (av != NULL && (av->get_collective() != av)) {
    blk_vars->push_back(const_cast<Variable*>(av->get_collective()));
    return;
  }
  blk_vars->push_back(v);
}

// void ExpressionAtomic::AddBlockVar(const Variable *v_i) {
//   Variable* v = const_cast<Variable*>(v_i);
//   if ((dynamic_cast<ArrayVariable*>(v)) && (v->get_collective() != v)) {
//     ArrayVariable * av = dynamic_cast<ArrayVariable*>(v);
// //     cout << "Adding collective var " << v->to_string() << endl;
//     GetBlockVars()->push_back(const_cast<Variable*>(av->get_collective()));
//   }
// //   cout << "Adding (const) " << v->to_string() << endl;
//   GetBlockVars()->push_back(v);
// }

void ExpressionAtomic::RemoveBlockVars(std::vector<Variable*> local_vars) {
  std::vector<Variable*>* blk_vars = GetBlockVars();
//   for (Variable* v : *blk_vars) 
//     cout << v->to_string() << "\n";
  
//   for (Variable* v : local_vars)
//     cout << v->to_string() << "\n";
  
//   for (Variable* v : *blk_vars) {
//     if (std::find(local_vars.begin(), local_vars.end(), v) == local_vars.end() &&
//           v->is_visible(b))
//       continue;
//     if (v->is_visible_local(b))
//       std::cout << v->to_string() << " Vis" << std::endl;
//     std::cout << "Removing " << v->to_string() << std::endl;
//     ArrayVariable* av = dynamic_cast<ArrayVariable*>(v);
//     if (av != NULL && av->get_collective() != av) {
// //       blk_vars->erase(std::remove(blk_vars->begin(), blk_vars->end(), av->get_collective()), blk_vars->end());
//       continue;
//     }
// //     blk_vars->erase(std::remove(blk_vars->begin(), blk_vars->end(), v), blk_vars->end());
//   }
  
  for (Variable* v : local_vars) {
    ArrayVariable* av = dynamic_cast<ArrayVariable*>(v);
    if (av != NULL && av->get_collective() != av) {
//       cout << "Removing collective var " << v->to_string() << endl;
      blk_vars->erase(std::remove(blk_vars->begin(), blk_vars->end(), av->get_collective()), blk_vars->end());
      continue;
    }
//     cout << "Removing var " << v->to_string() << endl;
    blk_vars->erase(std::remove(blk_vars->begin(), blk_vars->end(), v), blk_vars->end());
  }
} 

void ExpressionAtomic::ParseBlockVars(Block* if_true) {
  cout << if_true->stm_id << " --- ";
  for (Variable* v : *block_vars)
    cout << v->to_string() << endl;
  cout << endl;
}

void ExpressionAtomic::Output(std::ostream& out) const {
  // Need the '&' as atomic expressions expect a pointer to parameter p.
  out << GetOpName() << "(&";
  global_var_->Output(out);
  switch(atomic_type_) {
    case kDec:
    case kInc: break;
    case kAdd:
    case kSub:
    case kXchg:
    case kMin:
    case kMax:
    case kAnd:
    case kOr:
    case kXor: { out << ", " << val_; break; }
    case kCmpxchg: { out << ", " << cmp_ << ", " << val_; break; }  
    default: assert(0); // should never get here.
  }
  out << ")";
}

string ExpressionAtomic::GetOpName() const {
  switch(atomic_type_) {
    case kAdd:     return "atomic_add";
    case kSub:     return "atomic_sub";
    case kXchg:    return "atomic_xchg";
    case kInc:     return "atomic_inc";
    case kDec:     return "atomic_dec";
    case kCmpxchg: return "atomic_cmpxchg";
    case kMin:     return "atomic_min";
    case kMax:     return "atomic_max";
    case kAnd:     return "atomic_and";
    case kOr:      return "atomic_or";
    case kXor:     return "atomic_xor";
    default:       assert(0); // should never get here.
  }
}

}