#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <list>

#include "Checkers/UnusedMembersChecker.h"
#include "Checkers/AccessLevelChecker.h"
#include "Checkers/DeadCodeChecker.h"
#include "Checkers/SameConditionsChecker.h"
#include "Checkers/ConditionChecker.h"

using namespace std;

int main(int argc, char* argv[])
{  
  CXIndex index = clang_createIndex(0, 0);    
  CXTranslationUnit tUnit = clang_parseTranslationUnit(index, 0, argv, 
      argc, 0, 0, CXTranslationUnit_None);
     /* 
  for (int i = 0, n = clang_getNumDiagnostics(tUnit); i != n; ++i) {
    CXDiagnostic diag = clang_getDiagnostic(tUnit, i);
    CXString str = clang_formatDiagnostic(diag, 
        clang_defaultDiagnosticDisplayOptions());
    cout << clang_getCString(str) << endl;
    //fprintf(stderr, "%s\n", clang_getCString(str));    
    clang_disposeString(str);
  }*/
  
  CXCursor cursor = clang_getTranslationUnitCursor(tUnit);
  CXClientData data;

  clang_visitChildren(cursor, ConditionChecker::Check, &data);
  cout << ConditionChecker::GetDiagnostics() << endl;
  
  clang_visitChildren(cursor, DeadCodeChecker::Check, &data);
  cout << DeadCodeChecker::GetDiagnostics() << endl;

  clang_visitChildren(cursor, SameConditionsChecker::Check, &data);
  cout << SameConditionsChecker::GetDiagnostics() << endl;


  clang_visitChildren(cursor, AccessLevelChecker::Check, &data);
  cout << AccessLevelChecker::GetDiagnostics() << endl;

  clang_visitChildren(cursor, UnusedMembersChecker::Check, &data);
  cout << UnusedMembersChecker::GetDiagnostics() << endl;

  clang_disposeTranslationUnit(tUnit);
  clang_disposeIndex(index);
  return 0;
}