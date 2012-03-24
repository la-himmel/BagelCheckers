#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <list>

#include "Checkers.h"

using namespace std;

//---- Some bad stuff that I'm trying to catch ----
class Checker 
{
public:
  void Something() {
    bool a = 5 > 6; //condition that is never true
    if (a) {
      cout << "hello dead code";
    }

    if (false) {
    cout << "";
    } //condition that is never true
    }

private:
  void HeyHey() { //unused private method
   cout << "hey hey" << endl; 
   return;
   cout << "meow"; //dead code
  };
  
  int dummy;  //unused private field
};
//-------

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
  
  CXCursor cur = clang_getTranslationUnitCursor(tUnit);
  CXClientData data;

  //checking lowercase class name 
  clang_visitChildren(cur, VisitLowercaseClassName, &data); 
   
    
  
  clang_disposeTranslationUnit(tUnit);
  clang_disposeIndex(index);
  return 0;
}