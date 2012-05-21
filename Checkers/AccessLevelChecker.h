#ifndef _ACCESSLEVELCHECKER_H_
#define _ACCESSLEVELCHECKER_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>

#include "ConstAndUtilities.h"

using namespace std;

class AccessLevelChecker 
{
public:
  static enum CXChildVisitResult Check(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
  static string GetDiagnostics();
  static string GetStatistics();
  static void Reset();
  
private:
  static enum CXChildVisitResult FindClassName(CXCursor cursor,
    CXCursor parent, CXClientData client_data);

  static FileSection fileSection_;

  static AccessSection accessSection_;
  static AccessSection accessSectionBase_;

  static bool inherited_;

  static CXCursor overriding_;
  static bool searching_;

  static string baseClass_;
  static string subclass_;
  static string method_;

  static int count_;

  static string diagnostics_;
};

int AccessLevelChecker::count_ = 0;

AccessSection AccessLevelChecker::accessSection_ = ACCESS_OTHER;
AccessSection AccessLevelChecker::accessSectionBase_ = ACCESS_OTHER;

FileSection AccessLevelChecker::fileSection_ = SECTION_OTHER;
CXCursor AccessLevelChecker::overriding_ = clang_getNullCursor();

bool AccessLevelChecker::inherited_ = false;
bool AccessLevelChecker::searching_ = false;

string AccessLevelChecker::baseClass_ = "";
string AccessLevelChecker::subclass_ = "";
string AccessLevelChecker::method_ = "";
string AccessLevelChecker::diagnostics_ = "";

string AccessLevelChecker::GetDiagnostics() 
{  
  return AccessLevelChecker::diagnostics_;
}

string AccessLevelChecker::GetStatistics() 
{  
  string stat = "AL: " + intToString(count_) + "\n";
  return stat;  
}

enum CXChildVisitResult AccessLevelChecker::FindClassName(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
 
  if (clang_getCursorKind(cursor) == CXCursor_CXXAccessSpecifier) {        
    if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPrivate) { 
      if (searching_) {
        AccessLevelChecker::accessSectionBase_ = ACCESS_PRIVATE;
      } else {
        AccessLevelChecker::accessSection_ = ACCESS_PRIVATE;
      }      

    } else if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic) {
      if (searching_) {
        AccessLevelChecker::accessSectionBase_ = ACCESS_PUBLIC;
      } else {
        AccessLevelChecker::accessSection_ = ACCESS_PUBLIC;
      }      
      
    } else {
      if (searching_) {
        AccessLevelChecker::accessSectionBase_ = ACCESS_OTHER;
      } else {
        AccessLevelChecker::accessSection_ = ACCESS_OTHER;
      }      
    }
  } 

  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    if (searching_) {
      if (clang_equalCursors(cursor, AccessLevelChecker::overriding_)) {
        if (accessSectionBase_ == ACCESS_PRIVATE &&
          accessSection_ != ACCESS_PRIVATE) {

          string access = accessSection_ == ACCESS_PUBLIC ? "public" : 
              "protected";

          string diag = "Method '";
          diag.append(AccessLevelChecker::method_);
          diag.append("' is declared as private in base class '");
          diag.append(AccessLevelChecker::baseClass_);
          diag.append("' and is overrided as ");
          diag.append(access);
          diag.append(" in subclass '");
          diag.append(AccessLevelChecker::subclass_);
          diag.append("'. Are you sure you want to do this?\n");

          count_++;

          AccessLevelChecker::diagnostics_.append(diag);
          Reset();
        }
      }              
    } else {
      AccessLevelChecker::method_ = 
        clang_getCString(clang_getCursorSpelling(cursor));

      CXCursor *overriden = 0;
      unsigned num = 5;
      clang_getOverriddenCursors(cursor, &overriden, &num);

      if (num) {
        CXCursor c;
        for (int i = 0; i < num; i++) {
          CXCursor cur = *(overriden +i);

          c = clang_getCursorSemanticParent(cur);
          AccessLevelChecker::baseClass_ = 
            clang_getCString(clang_getCursorSpelling(c));

          AccessLevelChecker::searching_ = true;
          AccessLevelChecker::overriding_ = cur;

          CXClientData data;
          clang_visitChildren(c, AccessLevelChecker::FindClassName, &data); 

        }
        clang_disposeOverriddenCursors (overriden);
      }
    }
  } 
  
  return CXChildVisit_Recurse;
}

enum CXChildVisitResult AccessLevelChecker::Check(CXCursor cursor, 
  CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {    
    return CXChildVisit_Break;
  }

  if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
    if (ToyNavigator::IsInteresting(cursor)) {
      AccessLevelChecker::fileSection_ = SECTION_CLASS;
      AccessLevelChecker::inherited_ = false;
        AccessLevelChecker::subclass_ = 
            clang_getCString(clang_getCursorSpelling(cursor));
    
      CXClientData data;
      clang_visitChildren(cursor, AccessLevelChecker::FindClassName, &data); 
    }
  }
  return CXChildVisit_Continue;
}

void AccessLevelChecker::Reset()
{
  AccessLevelChecker::accessSection_ = ACCESS_OTHER;
  AccessLevelChecker::accessSectionBase_ = ACCESS_OTHER;

  AccessLevelChecker::fileSection_ = SECTION_OTHER;
  AccessLevelChecker::overriding_ = clang_getNullCursor();

  AccessLevelChecker::inherited_ = false;
  AccessLevelChecker::searching_ = false;

  AccessLevelChecker::baseClass_ = "";
  AccessLevelChecker::subclass_ = "";
  AccessLevelChecker::method_ = "";
}

#endif