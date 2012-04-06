#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <list>

// #include "UnusedMembersChecker.h"
// #include "AccessLevelChecker.h"
// #include "DeadCodeChecker.h"
#include "SameConditionsChecker.h"
#include "ConditionChecker.h"

using namespace std;

class P {
public:
  void DoSmth();
};

void P::DoSmth() 
{
  int a = 5, b = 6, c = 7, d = 8, e = 7, f = 10;

  if (a == b) {
    if (a == b) {
      c = 56;
    }

    if (a < a + 5)
      cout << "5" << endl;
  } else {
    if (a == b)
      cout << "same" << endl;    
    else {
      if (e < f) {
        d = 9;
      }
      if (e < f) {
        d = 10;
      }
    }
  }
  if (a == b)
    cout << "you repeat yourself" << endl;
}

int main(int argc, char* argv[])
{  
  CXIndex index = clang_createIndex(0, 0);    
  CXTranslationUnit tUnit = clang_parseTranslationUnit(index, 0, argv, 
      argc, 0, 0, CXTranslationUnit_None);
      
  for (int i = 0, n = clang_getNumDiagnostics(tUnit); i != n; ++i) {
    CXDiagnostic diag = clang_getDiagnostic(tUnit, i);
    CXString str = clang_formatDiagnostic(diag, 
        clang_defaultDiagnosticDisplayOptions());
    cout << clang_getCString(str) << endl;
    //fprintf(stderr, "%s\n", clang_getCString(str));    
    clang_disposeString(str);
  }
  
  CXCursor cursor = clang_getTranslationUnitCursor(tUnit);
  CXClientData data;
  clang_visitChildren(cursor, ConditionChecker::Check, &data);
  // clang_visitChildren(cursor, UnusedMembersChecker::Check, &data);
  cout << ConditionChecker::GetDiagnostics() << endl;
  
  //clang_visitChildren(cursor, DeadCodeChecker::Check, &data);
  
  clang_disposeTranslationUnit(tUnit);
  clang_disposeIndex(index);
  return 0;
}