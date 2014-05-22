// Placeholder for the inevitable.

#ifndef _CLSMITH_CLSTATEMENT_H_
#define _CLSMITH_CLSTATEMENT_H_

#include "CommonMacros.h"
#include "Statement.h"

namespace CLSmith {

// Will be the root of all OpenCL statements.
class CLStatement : public Statement {
 public:
  // Dynamic type info.
  enum CLStatementType {
    kBarrier = 0
  };

  explicit CLStatement(CLStatementType type, Block *block)
      : Statement(eCLStatement, block),
      cl_statement_type_(type) {
  }
  CLStatement(CLStatement&& other) = default;
  CLStatement& operator=(CLStatement&& other) = default;
  virtual ~CLStatement() {}

 private:
  CLStatementType cl_statement_type_;

  DISALLOW_COPY_AND_ASSIGN(CLStatement);
};

}  // namespace CLSmith

#endif  // _CLSMITH_CLSTATEMENT_H_
