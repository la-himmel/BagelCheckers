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
private:
  static enum CXChildVisitResult FindPrivateItems(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static enum CXChildVisitResult FindClassName(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindRefsAndCalls(CXCursor cursor,
    CXCursor parent, CXClientData client_data); 

  static string GetEntry(CXCursor cursor);

  static FileSection fileSection_;
  static AccessSection accessSection_;

  static string currentClass_;

  static vector<string> methods_;
  static map<string, int> fields_;

  int myDummy_;
};

string UnusedMembersChecker::currentClass_ = "";
FileSection UnusedMembersChecker::fileSection_ = SECTION_OTHER;
AccessSection UnusedMembersChecker::accessSection_ = ACCESS_OTHER;

vector<string> UnusedMembersChecker::methods_ = vector<string>();
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
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_MemberRefExpr) {
    PrintSpelling(cursor);

    string entry = GetEntry(cursor);
    vector<string>::iterator it = std::find(methods_.begin(), 
      methods_.end(), entry);

    if (it != methods_.end()) {
      methods_.erase(it);
    }
    else {
      map<string, int>::iterator it = fields_.find(entry);

      if (it != UnusedMembersChecker::fields_.end()) {
        it->second++;
        if (it->second > 1)
          fields_.erase(it);
      }      
    }
  }
   
  return CXChildVisit_Recurse;
}

string UnusedMembersChecker::GetDiagnostics() 
{
  string diag;
  for (map<string, int>::iterator it = UnusedMembersChecker::fields_.begin(); 
    it != UnusedMembersChecker::fields_.end(); ++it) {
      if (it->second == 0) {
        diag.append(it->first);
        diag.append(" field is UnusedMembersChecker.\n\n");
      } else if (it->second == 1) {
        diag.append(it->first);
        diag.append(" can be a local variable.\n\n");
      }
  } 
  for (vector<string>::iterator it = methods_.begin(); 
      it != methods_.end(); ++it) {
    diag.append(*it);
    diag.append(" method is UnusedMembersChecker.\n\n");
  }
  return diag;
}

enum CXChildVisitResult UnusedMembersChecker::FindClassName(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_TypeRef) {
    PrintSpelling(cursor);
    
  }
  if (clang_getCursorKind(cursor) == CXCursor_DeclStmt) {
    PrintSpelling(cursor);
  } 
  return CXChildVisit_Recurse;
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
    PrintSpelling(cursor);
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    PrintSpelling(cursor);
    if (UnusedMembersChecker::accessSection_ == ACCESS_PRIVATE) {
      string entry = GetEntry(cursor);
      UnusedMembersChecker::methods_.push_back(entry);
    }
  }
  if (clang_getCursorKind(cursor) == CXCursor_FieldDecl) {
    PrintSpelling(cursor);
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

enum CXChildVisitResult UnusedMembersChecker::Check(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
    
  if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
    PrintSpelling(cursor);
    UnusedMembersChecker::currentClass_ = clang_getCString(clang_getCursorSpelling(cursor));
    UnusedMembersChecker::fileSection_ = SECTION_CLASS;
    CXClientData data;
    clang_visitChildren(cursor, UnusedMembersChecker::FindPrivateItems, &data); 
  }

  if (clang_getCursorKind(cursor) == CXCursor_CallExpr) {
    PrintSpelling(cursor);
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    PrintSpelling(cursor);

    CXClientData data;
    clang_visitChildren(cursor, UnusedMembersChecker::FindClassName, &data);
    clang_visitChildren(cursor, UnusedMembersChecker::FindRefsAndCalls, &data); 
  }

  return CXChildVisit_Continue;
}

#endif