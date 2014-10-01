// Represents a barrier in the output code. Handles all the surrounding code
// that ensures the barrier has not been misplaced.
// TODO: Use actual statements for outputting.

#ifndef _CLSMITH_STATEMENTBARRIER_H_
#define _CLSMITH_STATEMENTBARRIER_H_

#include <fstream>
#include <memory>
#include <vector>

#include "CLSmith/CLStatement.h"
#include "CLSmith/MemoryBuffer.h"
#include "CommonMacros.h"

class Block;
class Expression;
class FactMgr;
namespace CLSmith {
class Divergence;
class Globals;
}  // namespace CLSmith

namespace CLSmith {

// Contains multiple statements and variables that control a single barrier.
// The contents are specific to a single barrier, and should not be modified by
// or affect another part of the program.
// These should be generated after the program has been created, and inserted
// where appropriate.
class StatementBarrier : public CLStatement {
 public:
  StatementBarrier(Block *block)
      : CLStatement(kBarrier, block), gate_(NULL), buffer_() {
  }
  StatementBarrier(Block *block, Variable *gate, MemoryBuffer *buffer)
      : CLStatement(kBarrier, block), gate_(gate), buffer_(buffer) {
  }
  StatementBarrier(StatementBarrier&& other) = default;
  StatementBarrier& operator=(StatementBarrier&& other) = default;
  virtual ~StatementBarrier() {}

  // Similar to the other statement factories, except this is not being made in
  // any specific context, so no CGContext object is needed.
  static StatementBarrier *make_random(Block *block);

  // Pure virtual in Statement. Not really needed.
  void get_blocks(std::vector<const Block *>& blks) const {}
  void get_exprs(std::vector<const Expression *>& exps) const {}

  // Outputs a gated block of code. The code reads and mutates its buffer item,
  // hits a barrier then assigns it to the paired threads item.
  void Output(std::ostream& out, FactMgr *fm, int indent) const;

  // Outputs a simple barrier that blocks every thread (local and global).
  static void OutputBarrier(std::ostream& out) {
    out << "barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);";
  }

  Variable *GetGate() { return gate_; }
  MemoryBuffer *GetBuffer() { return buffer_.get(); }

 private:
  Variable *gate_;
  std::unique_ptr<MemoryBuffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(StatementBarrier);
};

// Generates and sets gated barrier statements into parts of the program.
// A barrier is placed at the end of each section that has been marked as
// possibly divergent.
// TODO Put in another namespace. Make divergence const.
void GenerateBarriers(Divergence *divergence, Globals *globals);

}  // namespace CLSmith

#endif  // _CLSMITH_STATEMENTBARRIER_H_
