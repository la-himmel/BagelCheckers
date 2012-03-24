#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <map>

using namespace std;

void PrintSpelling(CXCursor cursor)
{
  string str = clang_getCString(clang_getCursorSpelling(cursor));
  // string name = clang_getCString(clang_getCursorDisplayName(cursor));
  cout << str << endl;
}

enum AccessSection { ACCESS_PRIVATE, ACCESS_OTHER, ACCESS_NONE };
enum FileSection { SECTION_METHOD, SECTION_CLASS, SECTION_OTHER };

class Unused 
{
public:
  static enum CXChildVisitResult VisitDeclarations(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);
private:
  static enum CXChildVisitResult FindPrivateItems(CXCursor cursor, 
    CXCursor parent, CXClientData client_data);

  static enum CXChildVisitResult FindRefsAndCalls(CXCursor cursor,
    CXCursor parent, CXClientData client_data); 

  static FileSection fileSection_;
  static AccessSection accessSection_;

  static string currentClass_;

  static map<string, vector<string> > methods_;
  static map<string, vector<string> > fields_;

  int myDummy_;
};

string Unused::currentClass_ = "";
FileSection Unused::fileSection_ = SECTION_OTHER;
AccessSection Unused::accessSection_ = ACCESS_NONE;

map<string, vector<string> > Unused::methods_ = map<string, vector<string> >();
map<string, vector<string> > Unused::fields_ = map<string, vector<string> >();

enum CXChildVisitResult Unused::FindRefsAndCalls(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  cout << "find refs and calls" << endl;
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
  
  if (clang_getCursorKind(cursor) == CXCursor_MemberRefExpr) {
    cout << "member ref expr inside the method! -- ";
    PrintSpelling(cursor);
     //Solve class name problem and replace this with proper code
      // Unused::methods_[currentClass_].push_back(
      //   clang_getCString(clang_getCursorSpelling(cursor)));
      // vector<string> vect = methods_[currentClass_];
      // cout << "vector size " << vect.size() << endl; 
    
  }
  if (clang_getCursorKind(cursor) == CXCursor_CallExpr) {
    cout << "field inside a method! -- ";
    PrintSpelling(cursor);
    //Problem: here we need to get class name to check the method
  } 
  return CXChildVisit_Recurse;
}

enum CXChildVisitResult Unused::FindPrivateItems(CXCursor cursor,
 CXCursor parent, CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXAccessSpecifier) {
    if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPrivate) { 
      Unused::accessSection_ = ACCESS_PRIVATE;
      cout << "private: -- ";
    } else {
      Unused::accessSection_ = ACCESS_OTHER;
      cout << "public or protected -- ";
    }
    PrintSpelling(cursor);
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    cout << "method -- ";
    PrintSpelling(cursor);
    if (Unused::accessSection_ == ACCESS_PRIVATE) {
      Unused::methods_[currentClass_].push_back(
        clang_getCString(clang_getCursorSpelling(cursor)));
      vector<string> vect = methods_[currentClass_];
      cout << "vector size " << vect.size() << endl; 
    }
  }
  if (clang_getCursorKind(cursor) == CXCursor_FieldDecl) {
    cout << "field ! -- ";
    PrintSpelling(cursor);
    if (Unused::accessSection_ == ACCESS_PRIVATE) {
      Unused::fields_[currentClass_].push_back(
        clang_getCString(clang_getCursorSpelling(cursor)));
      vector<string> vect = fields_[currentClass_];
      cout << "vector size " << vect.size() << endl; 
    }
  }
  return CXChildVisit_Continue;
}

enum CXChildVisitResult Unused::VisitDeclarations(CXCursor cursor, CXCursor parent, 
    CXClientData client_data) 
{
  if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {
    cout << "break\n" ;
    return CXChildVisit_Break;
  }
    
  if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
    cout << "class declaration -- ";
    PrintSpelling(cursor);
    Unused::currentClass_ = clang_getCString(clang_getCursorSpelling(cursor));
    Unused::fileSection_ = SECTION_CLASS;
    CXClientData data;
    clang_visitChildren(cursor, Unused::FindPrivateItems, &data); 
  }

  if (clang_getCursorKind(cursor) == CXCursor_CallExpr) {
    cout << "call -- ";
    PrintSpelling(cursor);
  }
  if (clang_getCursorKind(cursor) == CXCursor_CXXMethod) {
    cout << "method -- ";
    PrintSpelling(cursor);
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXClientData data;
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