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

#include "Checkers/ConstAndUtilities.h"

using namespace std;

int main(int argc, char* argv[])
{  
  CXIndex index = clang_createIndex(1, 0);    
  CXTranslationUnit tUnit = clang_parseTranslationUnit(index, 0, argv, 
      argc, 0, 0, CXTranslationUnit_None);
  string str = argv[1];

  ToyNavigator::SetFile(str);
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

  string diag;

  // clang_visitChildren(cursor, ConditionChecker::Check, &data);
  // diag.append(ConditionChecker::GetDiagnostics());
  // if (diag.size()) {
  //   diag.append("\n");  
  // }  
  
  clang_visitChildren(cursor, DeadCodeChecker::Check, &data);
  diag.append(DeadCodeChecker::GetDiagnostics());
  if (diag.size()) {
    diag.append("\n");  
  }

  // clang_visitChildren(cursor, SameConditionsChecker::Check, &data);
  // diag.append(SameConditionsChecker::GetDiagnostics());
  // if (diag.size()) {
  //   diag.append("\n");  
  // }
  
  clang_visitChildren(cursor, AccessLevelChecker::Check, &data);
  diag.append(AccessLevelChecker::GetDiagnostics());
  if (diag.size()) {
    diag.append("\n");  
  }

  clang_visitChildren(cursor, UnusedMembersChecker::Check, &data);
  diag.append(UnusedMembersChecker::GetDiagnostics());
  if (diag.size()) {
    diag.append("\n");  
    cout << diag << endl;
  } else {
    cout << "No diagnostics available for " << str << endl;
  }
  

  clang_disposeTranslationUnit(tUnit);
  clang_disposeIndex(index);
  return 0;
}