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

  static enum CXChildVisitResult FindBinOperator(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindChildren(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static void ChangeLevel(int step); 
  static void UpdateDiagnostics();

  static string first_;
  static string second_;

  static bool gotOne_;

  static int level_;

  static string diagnostics_;
};

string SameConditionsChecker::first_ = "";
string SameConditionsChecker::second_ = "";
string SameConditionsChecker::diagnostics_ = "";

bool SameConditionsChecker::gotOne_ = true;
int SameConditionsChecker::level_ = 0;

string SameConditionsChecker::GetDiagnostics() 
{  
  return SameConditionsChecker::diagnostics_;
}

void SameConditionsChecker::UpdateDiagnostics()
{
  string str = "Condition '";
  str.append(SameConditionsChecker::first_);
  str.append("' contains or is equal to condition '");
  str.append(SameConditionsChecker::second_);
  str.append("'. Are you sure?\n");
  SameConditionsChecker::diagnostics_.append(str);
}

enum CXChildVisitResult SameConditionsChecker::FindChildren(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    string str = GetText(cursor);

    if (!SameConditionsChecker::gotOne_) {
      SameConditionsChecker::first_ = str; 
      SameConditionsChecker::gotOne_ = true;

    } else {
      if (str == SameConditionsChecker::first_ || 
        SameConditionsChecker::first_.find(str) != string::npos) {
        SameConditionsChecker::second_ = str;
        SameConditionsChecker::UpdateDiagnostics();
      }

      SameConditionsChecker::ChangeLevel(-1);    

      SameConditionsChecker::gotOne_ = false;
      SameConditionsChecker::first_ = "";
      SameConditionsChecker::second_ = "";
    }
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
  
    CXClientData data;
    clang_visitChildren(cursor, SameConditionsChecker::FindBinOperator, &data); 
  
  } 

  return CXChildVisit_Recurse;
}

void SameConditionsChecker::ChangeLevel(int step) 
{
  if (step > 0)
    SameConditionsChecker::level_++;
  if (step < 0)
    SameConditionsChecker::level_--;
  if (step == 0)
    SameConditionsChecker::level_ = 1;
}

enum CXChildVisitResult SameConditionsChecker::FindBinOperator(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    SameConditionsChecker::ChangeLevel(1);
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
      (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)) 
  {
    CXClientData data;
    clang_visitChildren(cursor, SameConditionsChecker::FindStmts, &data); 
  } 
  
  return CXChildVisit_Continue;
}

#endif