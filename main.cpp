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

using namespace std;

string FormatDiag(string diag)
{  
  if (diag.size()) {
    diag.append("\n");
    // cout << diag << endl;
  }
  return diag;
}

void ProcessFile(int argc, char* argv[], char* filename)
{
  int k = 0;
  while (argv[k] != NULL) {
    // cout << argv[k++] << endl;   
    k++; //getting real number
  }

  CXIndex index = clang_createIndex(1, 0);  
  CXTranslationUnit tUnit = clang_parseTranslationUnit(index, filename, argv, 
      k, 0, 0, CXTranslationUnit_None);

  ToyNavigator::SetFile(filename);

  //clang diagnostics:
  for (int i = 0, n = clang_getNumDiagnostics(tUnit); i != n; ++i) {
    CXDiagnostic diag = clang_getDiagnostic(tUnit, i);
    CXString str = clang_formatDiagnostic(diag, 
        clang_defaultDiagnosticDisplayOptions());
    cout << clang_getCString(str) << endl;
    //fprintf(stderr, "%s\n", clang_getCString(str));    
    clang_disposeString(str);
  }


  //my diagnostics:
  CXCursor cursor = clang_getTranslationUnitCursor(tUnit);
  CXClientData data;

  string diag; 
  string temp;

  // clang_visitChildren(cursor, AccessLevelChecker::Check, &data); 
  // diag.append(FormatDiag(AccessLevelChecker::GetDiagnostics()));
  // diag.append(AccessLevelChecker::GetStatistics());

  // clang_visitChildren(cursor, ConditionChecker::Check, &data);
  // diag.append(FormatDiag(ConditionChecker::GetDiagnostics()));
  // diag.append(ConditionChecker::GetStatistics());
      
  // clang_visitChildren(cursor, DeadCodeChecker::Check, &data);
  // diag.append(FormatDiag(DeadCodeChecker::GetDiagnostics()));

  // clang_visitChildren(cursor, SameConditionsChecker::Check, &data);
  // diag.append(FormatDiag(SameConditionsChecker::GetDiagnostics()));
  // diag.append(SameConditionsChecker::GetStatistics());
  
  UnusedMembersChecker::Reset();
  clang_visitChildren(cursor, UnusedMembersChecker::Check, &data);
  diag.append(FormatDiag(UnusedMembersChecker::GetDiagnostics()));
  diag.append(UnusedMembersChecker::GetStatistics());

  if (diag.size()) { 
    diag.append("\n");  
    cout << diag << endl;
  } 

  clang_disposeTranslationUnit(tUnit); 
  clang_disposeIndex(index);
}

int main(int argc, char* argv[])
{  
  char* files[argc];
  char* args[argc];

  int j = 0;
  int i = 0;

  while (strncmp(argv[i], "$", 5)) {
    files[j++] = argv[i++];
  }
  files[j] = NULL;

  i++;
  int k = 0;
  // args[0] = argv[0];
  while (argv[i] != NULL) {
    args[k++] = argv[i++];    
  }
  args[k] = NULL;

  cout << "j: " << j << " i: " << i << endl;

  for (int l = 1; l < j; l++) {
    cout << "Processing file..." << files[l] << endl;
    ProcessFile(k +1, args, files[l]);
  }

  /*else {
    cout << "No diagnostics available for " << str << endl;
  }*/

  return 0;
}