#ifndef _SAMECONDITIONSCHECKER_H_
#define _SAMECONDITIONSCHECKER_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <cstdio>
#include <fstream>

#include "ConstAndUtilities.h"

using namespace std;

class SameConditionsChecker 
{
public:
  static enum CXChildVisitResult Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static string GetDiagnostics();

private:
  static enum CXChildVisitResult FindStmts(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindChildren(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static string GetText(CXCursor cursor);

  static string first_;

  static bool gotOne_;

  // static string diagnostics_;
};

string SameConditionsChecker::first_ = "";
bool SameConditionsChecker::gotOne_ = false;

string SameConditionsChecker::GetDiagnostics() 
{  
  string str;
  return str; //SameConditionsChecker::diagnostics_;
}

enum CXChildVisitResult SameConditionsChecker::FindChildren(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    
    if (!SameConditionsChecker::gotOne_) {
      SameConditionsChecker::first_ = SameConditionsChecker::GetText(cursor);
      SameConditionsChecker::gotOne_ = true;

      cout << "first: ";

    } else {
      cout << "second: ";
      SameConditionsChecker::gotOne_ = false;
    }
    string str = SameConditionsChecker::GetText(cursor);
    cout << "binary '" << str << "' " << endl;
  } 

  return CXChildVisit_Continue;
}

enum CXChildVisitResult SameConditionsChecker::FindStmts(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }

  if ((clang_getCursorKind(cursor) == CXCursor_IfStmt) || 
    (clang_getCursorKind(cursor) == CXCursor_WhileStmt)) {
    cout << "If while " << endl;
    CXClientData data;
    clang_visitChildren(cursor, SameConditionsChecker::FindStmts, &data); 
  } 

  if (clang_getCursorKind(cursor) == CXCursor_ForStmt) {
    cout << "for" << endl;
          // CXClientData data;
          // clang_visitChildren(c, SameConditionsChecker::FindClassName, &data); 
  } 
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    CXClientData data;
    clang_visitChildren(cursor, SameConditionsChecker::FindChildren, &data); 

  } 

  return CXChildVisit_Recurse;
}

enum CXChildVisitResult SameConditionsChecker::Check(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {    
    return CXChildVisit_Break;
  }
  if ((clang_getCursorKind(cursor) == CXCursor_CXXMethod) || 
    (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)) {

    CXClientData data;
    clang_visitChildren(cursor, SameConditionsChecker::FindStmts, &data); 
  } 
  return CXChildVisit_Continue;
}

string SameConditionsChecker::GetText(CXCursor cursor)
{
  CXSourceLocation loc = clang_getCursorLocation(cursor);
  CXFile file1;
  unsigned line, column, offset;
  clang_getSpellingLocation(loc, &file1, &line, &column, &offset);
  string filename = clang_getCString(clang_getFileName(file1));
  char *fn = (char *) clang_getCString(clang_getFileName(file1));

  CXSourceRange range = clang_getCursorExtent(cursor);

  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);

  unsigned stOffs, endOffs;
  clang_getSpellingLocation(start, &file1, &line, &column, &stOffs);
  clang_getSpellingLocation(end, &file1, &line, &column, &endOffs);

  FILE *file;
  file = fopen(fn, "r");
  fseek (file, offset, SEEK_SET);

  char *buffer = (char*) malloc (sizeof(char)*(endOffs -stOffs));
  fread (buffer, 1, (endOffs -stOffs), file);

  fclose (file);

  string str;
  str.assign(buffer);
  return str;
}

#endif