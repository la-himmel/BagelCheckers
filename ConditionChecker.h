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

  static enum CXChildVisitResult FindSameConds(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static string parentOperator_;
  static string lastCondition_;

  static bool divingInto_;
  static bool embedded_;

  static int level_;

  static string diagnostics_;

  static vector<string> conds_;
};

string ConditionChecker::diagnostics_ = "";

string ConditionChecker::parentOperator_ = "";
string ConditionChecker::lastCondition_ = "";

bool ConditionChecker::divingInto_ = false;
bool ConditionChecker::embedded_ = false;
int ConditionChecker::level_ = 0;

vector<string> ConditionChecker::conds_ = vector<string>();

string ConditionChecker::GetDiagnostics() 
{  
  return ConditionChecker::diagnostics_;
}

enum CXChildVisitResult ConditionChecker::FindBinOperator(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    if (ConditionChecker::divingInto_) {
      if (ConditionChecker::parentOperator_ == "") {
        parentOperator_ = GetText(cursor);
        if (!embedded_) {
          lastCondition_ = parentOperator_;
          return CXChildVisit_Break;
        }
      } else {
        if (parentOperator_ == GetText(cursor)) {
          string text = "This condition '";
          text.append(GetText(cursor));
          text.append("' is already there, are you sure?\n");
          diagnostics_.append(text);
        }
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
    ConditionChecker::divingInto_ = true;

    string lastCondition = lastCondition_;

    CXClientData data;
    clang_visitChildren(cursor, ConditionChecker::FindBinOperator, &data);

    if (lastCondition_ == lastCondition && lastCondition.size()) {
      string text= "This condition '";
      text.append(lastCondition);
      text.append("' is the same as previous, are you sure?\n");
      diagnostics_.append(text);
    }

    ConditionChecker::parentOperator_ = "";
    ConditionChecker::divingInto_ = false;
  
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
    (clang_getCursorKind(cursor) == CXCursor_WhileStmt) || 
    (clang_getCursorKind(cursor) == CXCursor_CompoundStmt)) {

    lastCondition_ = "";

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
    embedded_ = true;
    clang_visitChildren(cursor, ConditionChecker::FindStmts, &data); 

    embedded_ = false;
    clang_visitChildren(cursor, ConditionChecker::FindStmts, &data); 
  } 
 
  return CXChildVisit_Continue;
}

#endif