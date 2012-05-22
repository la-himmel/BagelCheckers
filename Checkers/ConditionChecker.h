#ifndef _CONDITIONSCHECKER_H_
#define _CONDITIONSCHECKER_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <cstdio>
#include <fstream>

#include "ConstAndUtilities.h"
#include "IChecker.h"

using namespace std;

class ConditionChecker : public IChecker
{
public:
  virtual void Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  virtual string GetDiagnostics();
  virtual string GetStatistics();
  virtual void Reset();
  virtual std::vector<CXCursorKind> GetInterestingCursors();

private:
  static enum CXChildVisitResult FindStmts(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindStmtsContinually(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindBinOperator(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindSameConds(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindReferences(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static string parentOperator_;
  static string lastCondition_;

  static CXCursor lastCondCursor_;

  static string lastBinaryExpr_;
  static string lastMember_;

  static bool divingInto_;
  static bool embedded_;

  static int level_;

  static int doubleVars_;
  static int contConds_;
  static int embConds_;

  static string diagnostics_;

  static vector<string> conds_;
  static vector<string> vars_;
};

string ConditionChecker::diagnostics_ = "";
string ConditionChecker::parentOperator_ = "";
string ConditionChecker::lastCondition_ = "";
string ConditionChecker::lastBinaryExpr_ = "";
string ConditionChecker::lastMember_ = "";

int ConditionChecker::doubleVars_ = 0;
int ConditionChecker::contConds_ = 0;
int ConditionChecker::embConds_ = 0;

CXCursor ConditionChecker::lastCondCursor_ = clang_getNullCursor();

bool ConditionChecker::divingInto_ = false;
bool ConditionChecker::embedded_ = false;
int ConditionChecker::level_ = 0;

vector<string> ConditionChecker::conds_ = vector<string>();
vector<string> ConditionChecker::vars_ = vector<string>();

string ConditionChecker::GetDiagnostics() 
{  
  return ConditionChecker::diagnostics_;
}

string ConditionChecker::GetStatistics()
{
  string stat = "Double variables: " + intToString(doubleVars_) + "\nContinual conditions: " 
    + intToString(contConds_) + " Embedded conditions: " + intToString(embConds_) + "\n";
  return stat;
}

enum CXChildVisitResult ConditionChecker::FindReferences(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_MemberRefExpr) {
    lastMember_ = GetText(cursor);
  }  

  if (clang_getCursorKind(cursor) == CXCursor_DeclRefExpr) {
    string ref = GetText(cursor);
    if (lastMember_.size()) {
      ref.append(lastMember_);
      lastMember_ = "";
    }

    if (std::find(vars_.begin(), vars_.end(), ref) != vars_.end()) {
      if (lastBinaryExpr_.size() && ref.size() && ref != "->") {
        string text = GetShortLocation(cursor);
        text.append(" This condition '");
        text.append(lastBinaryExpr_);
        text.append("' contains variable '");
        text.append(ref);
        text.append("' twice, don't you want to simplify it?\n");
        diagnostics_.append(text);
        doubleVars_++;
      }      
      
    } else {
      vars_.push_back(GetText(cursor));
    }  
  } 

  return CXChildVisit_Recurse;
}

enum CXChildVisitResult ConditionChecker::FindBinOperator(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    if (GetText(cursor).find(" = ") == string::npos) {
      lastBinaryExpr_ = GetText(cursor);
      vars_.clear();

      CXClientData data;
      clang_visitChildren(cursor, ConditionChecker::FindReferences, &data);
    }      

    if (ConditionChecker::divingInto_) {
      if (ConditionChecker::parentOperator_ == "") {
        parentOperator_ = GetText(cursor);
        if (!embedded_) {
          lastCondition_ = parentOperator_;
          lastCondCursor_ = cursor;

          return CXChildVisit_Break;
        }
      } else {
        if (parentOperator_ == GetText(cursor)) {
          string text = GetShortLocation(cursor);
          text.append("This condition '");
          text.append(GetText(cursor));
          text.append("' is already there, are you sure?\n");
          diagnostics_.append(text);
          embConds_++;
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
      (clang_getCursorKind(cursor) == CXCursor_WhileStmt)) 
  {
    ConditionChecker::divingInto_ = true;
    string lastCondition = lastCondition_;

    CXClientData data;
    clang_visitChildren(cursor, ConditionChecker::FindBinOperator, &data);

    if (lastCondition_ == lastCondition && lastCondition.size()) {
      string text = GetShortLocation(cursor); 
      text.append("This condition '");
      text.append(lastCondition);
      text.append(GetShortLocation(cursor));
      text.append("' is the same as previous, are you sure?\n");
      text.append(GetShortLocation(lastCondCursor_));
      diagnostics_.append(text);
      contConds_++;
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
      (clang_getCursorKind(cursor) == CXCursor_CompoundStmt)) 
  {
    lastCondition_ = "";
    lastCondCursor_ = clang_getNullCursor();

    CXClientData data;
    clang_visitChildren(cursor, ConditionChecker::FindStmtsContinually, &data); 
  }

  return CXChildVisit_Recurse;
}

std::vector<CXCursorKind> ConditionChecker::GetInterestingCursors()
{
  vector<CXCursorKind> cursors;
  cursors.push_back(CXCursor_CXXMethod);
  cursors.push_back(CXCursor_FunctionDecl);
  return cursors;
}

void ConditionChecker::Check(CXCursor cursor, 
  CXCursor parent, CXClientData client_data) 
{
  if ((clang_getCursorKind(cursor) == CXCursor_CXXMethod) || 
      (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)) 
  {
    if (ToyNavigator::IsInteresting(cursor)) {
      CXClientData data;
      embedded_ = true;
      clang_visitChildren(cursor, ConditionChecker::FindStmts, &data); 

      embedded_ = false;
      clang_visitChildren(cursor, ConditionChecker::FindStmts, &data); 
    }
  } 
}

void ConditionChecker::Reset()
{
  diagnostics_ = "";
  parentOperator_ = "";
  lastCondition_ = "";
  lastBinaryExpr_ = "";
  lastMember_ = "";

  // doubleVars_ = 0;
  // contConds_ = 0;
  // embConds_ = 0;

  lastCondCursor_ = clang_getNullCursor();

  divingInto_ = false;
  embedded_ = false;
  level_ = 0;

  conds_ = vector<string>();
  vars_ = vector<string>();
}

#endif