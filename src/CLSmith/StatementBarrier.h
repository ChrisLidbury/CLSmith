// Represents a barrier in the output code. Handles all the surrounding code
// that ensures the barrier has not been misplaced.

#ifndef _CLSMITH_STATEMENTBARRIER_H_
#define _CLSMITH_STATEMENTBARRIER_H_

#include <fstream>
#include <vector>

#include "CLSmith/CLStatement.h"
#include "CommonMacros.h"

class Block;
class Expression;
class FactMgr;

namespace CLSmith {

// Contains multiple statements and variables that control a single barrier.
// The contents are specific to a single barrier, and should not be modified by
// or affect another part of the program.
// These should be generated after the program has been created, and inserted
// where appropriate.
class StatementBarrier : public CLStatement {
 public:
  explicit StatementBarrier(Block *block) : CLStatement(kBarrier, block) {}
  StatementBarrier(StatementBarrier&& other) = default;
  StatementBarrier& operator=(StatementBarrier&& other) = default;
  virtual ~StatementBarrier() {}

  // Pure virtual in Statement. Not really needed.
  void get_blocks(std::vector<const Block *>& blks) const {}
  void get_exprs(std::vector<const Expression *>& exps) const {}

  void Output(std::ostream& out, FactMgr *fm = 0, int indent = 0) const;

  // TODO everything :<

 private:
  DISALLOW_COPY_AND_ASSIGN(StatementBarrier);
};

}  // namespace CLSmith

#endif  // _CLSMITH_STATEMENTBARRIER_H_
