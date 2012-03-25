#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <map>
#include <iterator>
#include <algorithm>

using namespace std;

class DeadCodeChecker 
{
public:
  static enum CXChildVisitResult Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static string GetDiagnostics();

private:
  static enum CXChildVisitResult FindDeadCode(CXCursor cursor,
    CXCursor parent, CXClientData client_data); 
  static enum CXChildVisitResult VisitChildren(CXCursor cursor,
    CXCursor parent, CXClientData client_data); 

  static bool foundReturnStmt_;

  static vector<string> methods_;
};

vector<string> DeadCodeChecker::methods_ = vector<string>();

bool DeadCodeChecker::foundReturnStmt_ = false;

enum CXChildVisitResult DeadCodeChecker::FindDeadCode(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    DeadCodeChecker::foundReturnStmt_ = false;
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_ReturnStmt) {
    DeadCodeChecker::foundReturnStmt_ = true;
  } else if (DeadCodeChecker::foundReturnStmt_) {
    cout << "( Found ) dead code: " << endl;

    CXSourceLocation location = clang_getCursorLocation(cursor);
    unsigned *line = new unsigned;
    unsigned *column = new unsigned;
    unsigned *offset = new unsigned;
    CXFile *file = new CXFile; 

    clang_getSpellingLocation(location, file, line, column, offset);
    if (file) {
      //TODO: fix a problem with return string and add diagnostics
      CXString filename = clang_getFileName(file);
      cout << clang_getCString(filename) << endl;
      cout << "line: " << *line << "column: " << *column << endl;

      clang_disposeString(filename);

    } 
  } 
  return CXChildVisit_Continue;
}

string DeadCodeChecker::GetDiagnostics() 
{
  string diag;

  return diag;
}

enum CXChildVisitResult DeadCodeChecker::VisitChildren(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_CompoundStmt) {
    DeadCodeChecker::foundReturnStmt_ = false;
    CXClientData data;
    clang_visitChildren(cursor, DeadCodeChecker::FindDeadCode, &data); 
  } 
  return CXChildVisit_Recurse;
}

enum CXChildVisitResult DeadCodeChecker::Check(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
    
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    // string str = clang_getCString(clang_getCursorSpelling(cursor));

    CXClientData data;
    clang_visitChildren(cursor, DeadCodeChecker::VisitChildren, &data); 
  }

  return CXChildVisit_Continue;
}