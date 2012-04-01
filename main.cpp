#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <list>

#include "UnusedMembersChecker.h"
#include "AccessLevelChecker.h"
#include "DeadCodeChecker.h"
#include "SameConditionsChecker.h"

using namespace std;

class K {
public:
  void func(int k, int j);
};

void f()
{
  int k = 456;
  int g = 546-90;

  if (k == g || k == g) {
    cout << "hey" << endl;
    int h = k - 50 + 97 *g;
    if (h < k && h > k && h < k)
      cout << "h" << endl;
  } 
  for (int i = 0; i < 1; i++) {
    cout << "h";
  }
}

void K::func(int k, int j) 
{
  if (k == j || k == j || k != j || k == j) {
    cout << "hey" << endl;
    int h = k - 50 + 97 *j;
    if (h < k && h > k && h < k)
      cout << "h" << endl;
  }
  if ( k < 967*j)
    cout << "fgdfg" << endl;
  int k1;
  while (k1 < 5 || k1 < 5) {
    k1 = 5;
  }
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
  clang_visitChildren(cursor, SameConditionsChecker::Check, &data);
  // clang_visitChildren(cursor, UnusedMembersChecker::Check, &data);
  // cout << AccessLevelChecker::GetDiagnostics() << endl;
  
  //clang_visitChildren(cursor, DeadCodeChecker::Check, &data);
  
  clang_disposeTranslationUnit(tUnit);
  clang_disposeIndex(index);
  return 0;
}