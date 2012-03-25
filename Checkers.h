#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <map>
#include <iterator>
#include <algorithm>

using namespace std;

void PrintSpelling(CXCursor cursor)
{
  string str = clang_getCString(clang_getCursorSpelling(cursor));
  // string name = clang_getCString(clang_getCursorDisplayName(cursor));
  //cout << str << endl;
}

enum AccessSection { ACCESS_PRIVATE, ACCESS_OTHER, ACCESS_NONE };
enum FileSection { SECTION_METHOD, SECTION_CLASS, SECTION_OTHER };

class Unused 
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

string Unused::currentClass_ = "";
FileSection Unused::fileSection_ = SECTION_OTHER;
AccessSection Unused::accessSection_ = ACCESS_NONE;

vector<string> Unused::methods_ = vector<string>();
map<string, int> Unused::fields_ = map<string, int>();

string Unused::GetEntry(CXCursor cursor) 
{
  string currentClass = Unused::currentClass_;
  return (currentClass.append("::")).append(
        clang_getCString(clang_getCursorSpelling(cursor)));
}

enum CXChildVisitResult Unused::FindRefsAndCalls(CXCursor cursor,
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

      if (it != Unused::fields_.end()) {
        it->second++;
        if (it->second > 1)
          fields_.erase(it);
      }      
    }
  }
   
  return CXChildVisit_Recurse;
}

string Unused::GetDiagnostics() 
{
  string diag;
  for (map<string, int>::iterator it = Unused::fields_.begin(); 
    it != Unused::fields_.end(); ++it) {
      if (it->second == 0) {
        diag.append(it->first);
        diag.append(" field is unused.\n\n");
      } else if (it->second == 1) {
        diag.append(it->first);
        diag.append(" can be a local variable.\n\n");
      }
  } 
  for (vector<string>::iterator it = methods_.begin(); 
      it != methods_.end(); ++it) {
    diag.append(*it);
    diag.append(" method is unused.\n\n");
  }
  return diag;
}

enum CXChildVisitResult Unused::FindClassName(CXCursor cursor,
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

enum CXChildVisitResult Unused::FindPrivateItems(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    return CXChildVisit_Break;
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXAccessSpecifier) {
    if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPrivate) { 
      Unused::accessSection_ = ACCESS_PRIVATE;
    } else {
      Unused::accessSection_ = ACCESS_OTHER;
    }
    PrintSpelling(cursor);
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    PrintSpelling(cursor);
    if (Unused::accessSection_ == ACCESS_PRIVATE) {
      string entry = GetEntry(cursor);
      Unused::methods_.push_back(entry);
    }
  }
  if (clang_getCursorKind(cursor) == CXCursor_FieldDecl) {
    PrintSpelling(cursor);
    if (Unused::accessSection_ == ACCESS_PRIVATE) {
      string entry = GetEntry(cursor);

      map<string, int>::iterator it = fields_.find(entry);

      if (it == Unused::fields_.end()) {
        Unused::fields_.insert(pair<string, int>(entry, 0));
      }

    }
  }
  return CXChildVisit_Continue;
}

enum CXChildVisitResult Unused::Check(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
    
  if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
    PrintSpelling(cursor);
    Unused::currentClass_ = clang_getCString(clang_getCursorSpelling(cursor));
    Unused::fileSection_ = SECTION_CLASS;
    CXClientData data;
    clang_visitChildren(cursor, Unused::FindPrivateItems, &data); 
  }

  if (clang_getCursorKind(cursor) == CXCursor_CallExpr) {
    PrintSpelling(cursor);
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    PrintSpelling(cursor);

    CXClientData data;
    clang_visitChildren(cursor, Unused::FindClassName, &data);
    clang_visitChildren(cursor, Unused::FindRefsAndCalls, &data); 
  }

  return CXChildVisit_Continue;
}

//Lowercase detection
static enum CXChildVisitResult DetectLowercaseClassName(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
    cout << "class declaration found!\n";

    string str = clang_getCString(clang_getCursorSpelling(cursor));
        
    if (str.at(0) >= 97 && str.at(0) <=122) {
      cout << "First class name letter is a small letter! class " 
           << str << endl;
    }
  }
  return CXChildVisit_Continue;
}