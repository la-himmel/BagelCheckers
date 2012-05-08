#ifndef _SAMECONDITIONSCHECKER_H_
#define _SAMECONDITIONSCHECKER_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "ConstAndUtilities.h"

using namespace std;

class SameConditionsChecker 
{
public:
  static enum CXChildVisitResult Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static string GetDiagnostics();
  static string GetStatistics();

private:
  static enum CXChildVisitResult FindStmts(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult VisitChildrenCont(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindBinOperator(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindChildren(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static void ChangeLevel(int step); 
  static void UpdateDiagnostics(CXCursor cursor);
  static void Reset();

  static string first_;
  static string second_;
  static string firstLoc_;
  static string secondLoc_;

  static bool searchingForCondition_;
  static string firstCond_;
  static string secondCond_;

  static CXCursor lastCondCont_;

  static bool gotOne_;

  static int level_;

  static int count_;

  static string diagnostics_;
};

string SameConditionsChecker::first_ = "";
string SameConditionsChecker::second_ = "";
string SameConditionsChecker::diagnostics_ = "";
string SameConditionsChecker::firstLoc_ = "";
string SameConditionsChecker::secondLoc_ = "";

string SameConditionsChecker::firstCond_ = "";
string SameConditionsChecker::secondCond_ = "";

bool SameConditionsChecker::searchingForCondition_ = false;

CXCursor SameConditionsChecker::lastCondCont_ = clang_getNullCursor();

int SameConditionsChecker::count_ = 0;

bool SameConditionsChecker::gotOne_ = true;
int SameConditionsChecker::level_ = 0;

string SameConditionsChecker::GetDiagnostics() 
{  
  return SameConditionsChecker::diagnostics_;
}

string SameConditionsChecker::GetStatistics()
{
  string stat;
  if (count_) {
    stat = "NC: " + intToString(count_) + "\n";
  }
  
  return stat;  
}

void SameConditionsChecker::UpdateDiagnostics(CXCursor cursor)
{
   //i know this is very, very bad
  if (GetShortLocation(cursor) == SameConditionsChecker::firstLoc_)
    return;

  string str = GetShortLocation(cursor);
  str.append(SameConditionsChecker::firstLoc_);
  str.append("Condition '");
  str.append(SameConditionsChecker::first_);
  str.append("' contains or is equal to condition '");
  str.append(SameConditionsChecker::second_);
  str.append("'. Are you sure?\n");
  SameConditionsChecker::diagnostics_.append(str);
  count_++;
}

enum CXChildVisitResult SameConditionsChecker::FindChildren(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }

  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    // cout << "Level 4 (BO Children) : " << GetText(cursor) << endl; 

    string str = GetText(cursor);
    if (!str.size()) {
      return CXChildVisit_Continue;
    }

    if (!SameConditionsChecker::gotOne_) {
      SameConditionsChecker::first_ = str; 
      SameConditionsChecker::firstLoc_ = GetShortLocation(cursor);
      SameConditionsChecker::gotOne_ = true;

    } else {
      if (str == SameConditionsChecker::first_ || 
        SameConditionsChecker::first_.find(str) != string::npos) {
        SameConditionsChecker::secondLoc_ = GetShortLocation(cursor);
        SameConditionsChecker::second_ = str;
        SameConditionsChecker::UpdateDiagnostics(cursor);
      }

      SameConditionsChecker::ChangeLevel(-1);    

      Reset();
    }
  } 

  return CXChildVisit_Continue;
}


enum CXChildVisitResult SameConditionsChecker::FindBinOperator(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    // cout << "NULL STMT" << endl;
    return CXChildVisit_Break;
  }
    if (clang_getCursorKind(cursor) == CXCursor_CompoundStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_BinaryOperator) {
    SameConditionsChecker::ChangeLevel(1);

    string cursorText = GetText(cursor);

    if ((cursorText.find("<") != string::npos) || 
        (cursorText.find("<=") != string::npos) || 
        (cursorText.find(">") != string::npos) ||
        (cursorText.find(">=") != string::npos) ||
        (cursorText.find("==") != string::npos)) 
    {
      // cout << "Level 3 (binary operator) : " << cursorText << endl;

      CXClientData data;
      clang_visitChildren(cursor, SameConditionsChecker::FindChildren, &data); 
    }

  } 

  return CXChildVisit_Recurse;
}

enum CXChildVisitResult SameConditionsChecker::VisitChildrenCont(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }

  if (!searchingForCondition_ && ((clang_getCursorKind(cursor) == CXCursor_IfStmt) || 
    (clang_getCursorKind(cursor) == CXCursor_WhileStmt))) 
  {
    // cout << "while and if" << endl;
    if (clang_equalCursors(lastCondCont_, clang_getNullCursor())) {
      lastCondCont_ = cursor;
    } else {

      CXClientData data;
      searchingForCondition_ = true;
      firstCond_ = "";
      secondCond_ = "";

      clang_visitChildren(lastCondCont_, SameConditionsChecker::VisitChildrenCont, &data); 
      searchingForCondition_ = true;
      clang_visitChildren(cursor, SameConditionsChecker::VisitChildrenCont, &data); 
      if (firstCond_ == secondCond_) {
        cout << "WARNING: These conditions are equal:\n" << GetShortLocation(lastCondCont_) 
           << firstCond_ << "\n and: " << GetShortLocation(cursor) << secondCond_ << endl;
      }
      searchingForCondition_ = false;
      lastCondCont_ = cursor;
    }

  } else if (searchingForCondition_) {
    cout << "searching for conditions" << endl;
    if (!firstCond_.size()) {
      firstCond_ = GetText(cursor);
    } else if (!secondCond_.size()) {
      secondCond_ = GetText(cursor);
    }

    searchingForCondition_ = false;
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
    (clang_getCursorKind(cursor) == CXCursor_WhileStmt) ||
    (clang_getCursorKind(cursor) == CXCursor_CompoundStmt))  
  {
    //uncomment for one level checks
    // lastCondCont_ = clang_getNullCursor();
    CXClientData data;
    // clang_visitChildren(cursor, SameConditionsChecker::VisitChildrenCont, &data); 
    // lastCondCont_ = clang_getNullCursor();

    clang_visitChildren(cursor, SameConditionsChecker::FindBinOperator, &data);  
  } 

  return CXChildVisit_Recurse;
}

enum CXChildVisitResult SameConditionsChecker::Check(CXCursor cursor, 
  CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {    
    return CXChildVisit_Break;
  }
  if ((clang_getCursorKind(cursor) == CXCursor_CXXMethod) || 
      (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)) 
  {
    if (ToyNavigator::IsInteresting(cursor)) {
      lastCondCont_ = clang_getNullCursor();
      CXClientData data;
      Reset();
      //one level conditions:   
      // clang_visitChildren(cursor, SameConditionsChecker::VisitChildrenCont, &data); 
      clang_visitChildren(cursor, SameConditionsChecker::FindStmts, &data); 

      }
  }  
  return CXChildVisit_Continue;
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

void SameConditionsChecker::Reset()
{
  SameConditionsChecker::gotOne_ = false;
  SameConditionsChecker::first_ = "";
  SameConditionsChecker::second_ = "";
  SameConditionsChecker::firstLoc_ = "";
  SameConditionsChecker::secondLoc_ = "";
}

#endif