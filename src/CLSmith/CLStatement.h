// Root of all OpenCL statements.
// Will handle the creation of random OpenCL statements.
// TODO further implement functions in the Statement class
// (e.g. is_compound(t)).

#ifndef _CLSMITH_CLSTATEMENT_H_
#define _CLSMITH_CLSTATEMENT_H_

#include "CommonMacros.h"
#include "Statement.h"

class CGContext;

namespace CLSmith {

// Will be the root of all OpenCL statements.
class CLStatement : public Statement {
 public:
  // Dynamic type info.
  enum CLStatementType {
    kNone = 0,  // Sentinel value
    kBarrier,
    kEMI,
    kFakeDiverge,  // Gross hack alert
    kAtomic
  };

  CLStatement(CLStatementType type, Block *block)
      : Statement(eCLStatement, block),
      cl_statement_type_(type) {
  }
  CLStatement(CLStatement&& other) = default;
  CLStatement& operator=(CLStatement&& other) = default;
  virtual ~CLStatement() {}

  // Factory for creating a random OpenCL statement.
  static CLStatement *make_random(CGContext& cg_context,
      enum CLStatementType st);

  // Create an if statement with the test expression being uniform, but
  // appearing to diverge.
  static CLStatement *CreateFakeDivergentIf(CGContext& cg_context);

  // Initialise the probability table for selecting a random statement. Should
  // be called once on startup.
  static void InitProbabilityTable();

  // Getter the the statement type.
  enum CLStatementType GetCLStatementType() const { return cl_statement_type_; }

 private:
  CLStatementType cl_statement_type_;

  DISALLOW_COPY_AND_ASSIGN(CLStatement);
};

// Hook method for csmith.
Statement *make_random_st(CGContext& cg_context);

}  // namespace CLSmith

#endif  // _CLSMITH_CLSTATEMENT_H_
