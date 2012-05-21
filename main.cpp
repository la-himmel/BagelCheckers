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

void ProcessFile(int argc, char* argv[], char* filename)
{
  int k = 0;
  while (argv[k] != NULL) {
    k++; //getting real number
  }

  CXIndex index = clang_createIndex(1, 0);  
  CXTranslationUnit tUnit = clang_parseTranslationUnit(index, filename, argv, 
      k, 0, 0, CXTranslationUnit_None);

  ToyNavigator::SetFile(filename);

  for (int i = 0, n = clang_getNumDiagnostics(tUnit); i != n; ++i) {
    CXDiagnostic diag = clang_getDiagnostic(tUnit, i);
    CXString str = clang_formatDiagnostic(diag, 
        clang_defaultDiagnosticDisplayOptions());
    cout << clang_getCString(str) << endl;
    clang_disposeString(str);
  }

  CXCursor cursor = clang_getTranslationUnitCursor(tUnit);
  CXClientData data;

  // AccessLevelChecker::Run(cursor, &data); 
  // DeadCodeChecker::Run(cursor, &data); 
  ConditionChecker::Run(cursor, &data); 
  SameConditionsChecker::Run(cursor, &data);  
  // UnusedMembersChecker::Run(cursor, &data);

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
    cout << "Processing file...( " << l << " / " << j << " )" << files[l] << endl;
    ProcessFile(k +1, args, files[l]);
  }

  /*else {
    cout << "No diagnostics available for " << str << endl;
  }*/

  return 0;
}