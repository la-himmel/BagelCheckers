#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <list>

#include "UnusedMembersChecker.h"
#include "AccessLevelChecker.h"
// #include "Checkable.h"
#include "DeadCodeChecker.h"

using namespace std;

class A {
public:
  void print() { /* something */ }
private:
  virtual void func();
};

class B : public A {
public:
  virtual void func();
  int x;
};

class C : public A {
protected:
  virtual void func();
};

void A::func() 
{ 
  cout << "A, private" << endl; 
}

void B::func() 
{ 
  cout << "B, public" << endl; 
}

void C::func() 
{ 
  cout << "xc" << endl; 
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
  clang_visitChildren(cursor, AccessLevelChecker::Check, &data);
  // clang_visitChildren(cursor, UnusedMembersChecker::Check, &data);
  cout << AccessLevelChecker::GetDiagnostics() << endl;
  
  //clang_visitChildren(cursor, DeadCodeChecker::Check, &data);
  
  clang_disposeTranslationUnit(tUnit);
  clang_disposeIndex(index);
  return 0;
}