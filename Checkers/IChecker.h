#ifndef _ICHECKER_H_
#define _ICHECKER_H_

#include "Index.h"
#include <stdio.h>

class IChecker 
{
public:
  virtual void Check(CXCursor cursor, CXCursor parent, 
  	CXClientData client_data) = 0;
  virtual std::string GetDiagnostics() = 0;
  virtual std::string GetStatistics() = 0;
  virtual void Reset() = 0;
  virtual std::vector<CXCursorKind> GetInterestingCursors() = 0;
  virtual std::string GetName() = 0;
};

#endif