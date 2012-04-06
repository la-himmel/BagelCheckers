#ifndef _CONDITIONSCHECKER_H_
#define _CONDITIONSCHECKER_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <cstdio>
#include <fstream>

#include "ConstAndUtilities.h"

using namespace std;

class ConditionChecker 
{
public:
  static enum CXChildVisitResult Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static string GetDiagnostics();

private:
  static enum CXChildVisitResult FindStmts(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindStmtsContinually(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindBinOperator(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static void UpdateDiagnostics();

  static string GetText(CXCursor cursor);

  static string parentOperator_;

  static bool divingInto_;

  static int level_;

  static string diagnostics_;
};

string ConditionChecker::diagnostics_ = "";

string ConditionChecker::parentOperator_ = "";

bool ConditionChecker::divingInto_ = false;
int ConditionChecker::level_ = 0;

string ConditionChecker::GetDiagnostics() 
{  
  return ConditionChecker::diagnostics_;
}

void ConditionChecker::UpdateDiagnostics()
{
  string str = "Condition '";
  ConditionChecker::diagnostics_.append(str);
}

enum CXChildVisitResult ConditionChecker::FindBinOperator(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    cout << "bin operator" << GetText(cursor) << endl;
    if (ConditionChecker::divingInto_) {
      if (ConditionChecker::parentOperator_ == "") {
        parentOperator_ = GetText(cursor);
      } else {
        if (parentOperator_ == GetText(cursor))
          cout << "***** Diagnostics ***** this condition '" << GetText(cursor) 
               << "' is already there, are you sure?" << endl;
      } 
    }    
  } 

  return CXChildVisit_Recurse;
}

enum CXChildVisitResult ConditionChecker::FindStmtsContinually(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }

  if ((clang_getCursorKind(cursor) == CXCursor_IfStmt) || 
    (clang_getCursorKind(cursor) == CXCursor_WhileStmt)) {
    cout << "--------------------------------" << endl;
    cout << "Found if or while stmt" << endl;
    cout << ConditionChecker::GetText(cursor) << endl;
    ConditionChecker::divingInto_ = true;
  
    CXClientData data;
    clang_visitChildren(cursor, ConditionChecker::FindBinOperator, &data); 

    ConditionChecker::parentOperator_ = "";
    ConditionChecker::divingInto_ = true;
  
  } 

  return CXChildVisit_Continue;
}


enum CXChildVisitResult ConditionChecker::FindStmts(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }

  if ((clang_getCursorKind(cursor) == CXCursor_IfStmt) || 
    (clang_getCursorKind(cursor) == CXCursor_WhileStmt)) {
  
    CXClientData data;
    clang_visitChildren(cursor, ConditionChecker::FindStmtsContinually, &data); 
  
  } 
  if (clang_getCursorKind(cursor) == CXCursor_CompoundStmt) {
    CXClientData data;
    clang_visitChildren(cursor, ConditionChecker::FindStmtsContinually, &data); 
  }

  return CXChildVisit_Recurse;
}

enum CXChildVisitResult ConditionChecker::Check(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {    
    return CXChildVisit_Break;
  }
  if ((clang_getCursorKind(cursor) == CXCursor_CXXMethod) || 
    (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)) {
    CXClientData data;
    clang_visitChildren(cursor, ConditionChecker::FindStmts, &data); 
  } 
  return CXChildVisit_Continue;
}

string ConditionChecker::GetText(CXCursor cursor)
{
  CXFile file1;

  unsigned line, column, offset, stOffs, endOffset;
  clang_getSpellingLocation(clang_getCursorLocation(cursor), &file1, &line, &column, &offset);
  
  CXSourceLocation start = clang_getRangeStart(clang_getCursorExtent(cursor));
  CXSourceLocation end = clang_getRangeEnd(clang_getCursorExtent(cursor));

  clang_getSpellingLocation(start, &file1, &line, &column, &stOffs);
  clang_getSpellingLocation(end, &file1, &line, &column, &endOffset);

  FILE *file;
  char *fn = (char *) clang_getCString(clang_getFileName(file1));
  file = fopen(fn, "r");
  fseek (file, offset, SEEK_SET);
  char *buffer = (char*) malloc (sizeof(char)*(endOffset -stOffs));
  fread (buffer, 1, (endOffset -stOffs), file);

  fclose (file);

  string str;
  str.assign(buffer);
  return str;
}



#endif