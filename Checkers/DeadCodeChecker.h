#ifndef _DEADCODECHECKER_H_
#define _DEADCODECHECKER_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <sstream>

#include <algorithm>

using namespace std;

class DeadCodeChecker 
{
public:
  static void Run(CXCursor cursor, CXClientData client_data);

private:
  static enum CXChildVisitResult Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static string GetDiagnostics();

  static enum CXChildVisitResult FindDeadCode(CXCursor cursor,
    CXCursor parent, CXClientData client_data); 
  static enum CXChildVisitResult VisitChildren(CXCursor cursor,
    CXCursor parent, CXClientData client_data); 
  static void Reset();

  static bool foundReturnStmt_;
  static string diag_;

  static vector<string> methods_;
};

vector<string> DeadCodeChecker::methods_ = vector<string>();
string DeadCodeChecker::diag_ = "";

bool DeadCodeChecker::foundReturnStmt_ = false;

void DeadCodeChecker::Run(CXCursor cursor, CXClientData client_data) 
{
  Reset();
  clang_visitChildren(cursor, DeadCodeChecker::Check, &client_data);
  cout << FormatDiag(GetDiagnostics()) << /*GetStatistics() << */endl;
}

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
    
    CXSourceLocation location = clang_getCursorLocation(cursor);
    unsigned line;
    unsigned column;
    unsigned offset;
    CXFile file; 

    clang_getSpellingLocation(location, &file, &line, &column, &offset);
    if (file) {
      CXString filename = clang_getFileName(file);
      stringstream ss;
      ss << "Dead code detected! " << clang_getCString(filename)
           << " ln: " << line << " col: " << column << endl;
      string str = ss.str();

      clang_disposeString(filename);

      DeadCodeChecker::diag_.append(str);
    } 
  } 

  return CXChildVisit_Continue;
}

string DeadCodeChecker::GetDiagnostics() 
{
  return diag_;
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

enum CXChildVisitResult DeadCodeChecker::Check(CXCursor cursor, 
  CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
    
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    if (ToyNavigator::IsInteresting(cursor)) {
      CXClientData data;
      clang_visitChildren(cursor, DeadCodeChecker::VisitChildren, &data); 
    }
  }

  return CXChildVisit_Continue;
}

void DeadCodeChecker::Reset()
{
  methods_ = vector<string>();
  diag_ = "";
}


#endif