#ifndef _UNUSEDMEMBERCHECKER_H_
#define _UNUSEDMEMBERCHECKER_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <map>
#include <iterator>
#include <algorithm>

#include "ConstAndUtilities.h"

using namespace std;

class UnusedMembersChecker 
{
public:
  static enum CXChildVisitResult Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static string GetDiagnostics();
  static string GetStatistics();
  static void Reset();

private:
  static enum CXChildVisitResult FindPrivateItems(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static enum CXChildVisitResult FindRefsAndCalls(CXCursor cursor,
    CXCursor parent, CXClientData client_data); 

  static string GetEntry(CXCursor cursor);

  static FileSection fileSection_;
  static AccessSection accessSection_;

  static string currentClass_;

  static map<string, int> methods_;
  static map<string, int> fields_;

  static int localFields_;
  static int unusedMethods_;
  static int unusedVars_;
};

string UnusedMembersChecker::currentClass_ = "";
FileSection UnusedMembersChecker::fileSection_ = SECTION_OTHER;
AccessSection UnusedMembersChecker::accessSection_ = ACCESS_OTHER;

int UnusedMembersChecker::localFields_ = 0;
int UnusedMembersChecker::unusedVars_ = 0;
int UnusedMembersChecker::unusedMethods_ = 0;

map<string, int> UnusedMembersChecker::methods_ = map<string, int>();
map<string, int> UnusedMembersChecker::fields_ = map<string, int>();

string UnusedMembersChecker::GetEntry(CXCursor cursor) 
{
  string currentClass = UnusedMembersChecker::currentClass_;
  return (currentClass.append("::")).append(
        clang_getCString(clang_getCursorSpelling(cursor)));
}

enum CXChildVisitResult UnusedMembersChecker::FindRefsAndCalls(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_MemberRefExpr) {
    string entry = GetEntry(cursor);

    map<string, int>::iterator it = methods_.find(entry);

    if (it != UnusedMembersChecker::methods_.end()) {
        it->second++;       
        // cout << "Increased : " << endl;  
        // PrintSpelling(cursor);
    } else {
      map<string, int>::iterator it = fields_.find(entry);

      if (it != UnusedMembersChecker::fields_.end()) {
        it->second++;  
        // cout << "Increased : " << endl;  
        // PrintSpelling(cursor);
      } else {
        // cout << "Found MEMBER reference, not in list: -------- ";
        // PrintSpelling(cursor);
      }
    }       
  }   
  return CXChildVisit_Recurse;
}

string UnusedMembersChecker::GetStatistics()
{
  string stat = "UV: " + intToString(unusedVars_) + " UM: " 
    + intToString(unusedMethods_) + " LV: " + intToString(localFields_) + "\n";
  return stat;
}

string UnusedMembersChecker::GetDiagnostics() 
{
  string diag;
  for (map<string, int>::iterator it = UnusedMembersChecker::fields_.begin(); 
    it != UnusedMembersChecker::fields_.end(); ++it) {
      if (it->second == 0) {
        diag.append(it->first);
        diag.append(" field is unused.\n");
        unusedVars_++;
      } else if (it->second == 1) {
        diag.append(it->first);
        diag.append(" can be a local variable.\n");
        localFields_++;
      }
  } 

  for (map<string, int>::iterator it = UnusedMembersChecker::methods_.begin(); 
    it != UnusedMembersChecker::methods_.end(); ++it) 
  {
    if (it->second == 0) {
      diag.append(it->first);
      diag.append(" method is unused.\n");
      unusedMethods_++;
    }
  }

  return diag;
}

enum CXChildVisitResult UnusedMembersChecker::FindPrivateItems(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }

  if (clang_getCursorKind(cursor) == CXCursor_CXXAccessSpecifier) {
    if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPrivate) { 
      UnusedMembersChecker::accessSection_ = ACCESS_PRIVATE;
    } else {
      UnusedMembersChecker::accessSection_ = ACCESS_OTHER;
    }
  }

  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    // PrintSpelling(cursor);
    if (UnusedMembersChecker::accessSection_ == ACCESS_PRIVATE) {
      string entry = GetEntry(cursor);
      
      map<string, int>::iterator it = methods_.find(entry);

      if (it == UnusedMembersChecker::methods_.end()) {
        UnusedMembersChecker::methods_.insert(pair<string, int>(entry, 0));
      }
    }
  }

  if (clang_getCursorKind(cursor) == CXCursor_FieldDecl) {
    if (UnusedMembersChecker::accessSection_ == ACCESS_PRIVATE) {
      string entry = GetEntry(cursor);
      map<string, int>::iterator it = fields_.find(entry);

      if (it == UnusedMembersChecker::fields_.end()) {
        UnusedMembersChecker::fields_.insert(pair<string, int>(entry, 0));
      }
    }
  }
  return CXChildVisit_Continue;
}

enum CXChildVisitResult UnusedMembersChecker::Check(CXCursor cursor, 
  CXCursor parent, CXClientData client_data) 
{  
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
    
  if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
    if (ToyNavigator::IsInteresting(cursor)) {
      UnusedMembersChecker::currentClass_ = 
      clang_getCString(clang_getCursorSpelling(cursor));
      UnusedMembersChecker::fileSection_ = SECTION_CLASS;
      CXClientData data;
      clang_visitChildren(cursor, UnusedMembersChecker::FindPrivateItems, 
        &data); 
    }
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    if (ToyNavigator::IsInteresting(cursor)) {
      CXClientData data;
      clang_visitChildren(cursor, UnusedMembersChecker::FindRefsAndCalls, 
        &data); 
    }
  }

  return CXChildVisit_Continue;
}

void UnusedMembersChecker::Reset()
{
  currentClass_ = "";
  fileSection_ = SECTION_OTHER;
  accessSection_ = ACCESS_OTHER;

//let the stats be common
  // localFields_ = 0;
  // unusedVars_ = 0;
  // unusedMethods_ = 0;

  methods_ = map<string, int>();
  fields_ = map<string, int>();
}

#endif