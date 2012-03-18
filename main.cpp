#include "include/clang-c/Index.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <list>

#include <unistd.h> 
#include <dirent.h> 

using namespace std;

int GetFileNames(string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

class badClass 
{
public:
  int meow;  
};

class Checker 
{
public:
//string GetDiagnostics() { return ""; }
  
  string text;
  string filename;
};

static enum CXChildVisitResult Visit(CXCursor cursor, CXCursor parent, 
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

int main(int argc, char* argv[])
{  
  CXIndex index = clang_createIndex(0, 0);    
  CXTranslationUnit tUnit = clang_parseTranslationUnit(index, 0, argv, 
      argc, 0, 0, CXTranslationUnit_None);
      
  for (int i = 0, n = clang_getNumDiagnostics(tUnit); i != n; ++i) {
    CXDiagnostic diag = clang_getDiagnostic(tUnit, i);
    CXString str = clang_formatDiagnostic(diag, 
        clang_defaultDiagnosticDisplayOptions());
    cout << clang_getCString(str) << " hey meow " << endl;

    //fprintf(stderr, "%s\n", clang_getCString(str));
    
    clang_disposeString(str);
  }
  
  CXCursor cur = clang_getTranslationUnitCursor(tUnit);

  CXClientData data;
  clang_visitChildren(cur, Visit, &data);  
    
  char buffer[FILENAME_MAX];
  getcwd(buffer, FILENAME_MAX);
  
  string dir = buffer;
  dir.append("/checks/");    //current dir is string(".")

  vector<string> files = vector<string>();
  GetFileNames(dir, files);

  for (unsigned int i = 0;i < files.size();i++) {
      //cout << files[i] << endl;
  }

  clang_disposeTranslationUnit(tUnit);
  clang_disposeIndex(index);
  return 0;
}