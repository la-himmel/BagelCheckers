#include "Index.h"

#include <iostream>
#include <stdio.h>

using namespace std;

static enum CXChildVisitResult VisitLowercaseClassName(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
    cout << "class declaration found!\n";

    string str = clang_getCString(clang_getCursorSpelling(cursor));
        
    if (str.at(0) >= 97 && str.at(0) <=122) {
      cout << "First class name letter is a small letter! class " 
           << str << endl;
    }
  }

  return CXChildVisit_Continue;
}