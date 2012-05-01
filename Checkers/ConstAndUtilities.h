#ifndef _CONSTANDUTILITIES_H_
#define _CONSTANDUTILITIES_H_

#include "Index.h"

#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace std;

enum AccessSection { ACCESS_PRIVATE, ACCESS_PUBLIC, ACCESS_PROTECTED, ACCESS_OTHER };
enum FileSection { SECTION_METHOD, SECTION_CLASS, SECTION_OTHER };

class ToyNavigator
{
public:
  static bool IsInteresting(CXCursor);
  static void SetFile(string);

  static string source_;
  static string header_;
};

string ToyNavigator::source_ = "";
string ToyNavigator::header_ = "";

void ToyNavigator::SetFile(string str)
{
  size_t found = str.find_last_of("/\\");
  source_ = str.substr(found +1);
  found = source_.find("cpp");
  if (found != string::npos) {
    header_ = source_.substr(0, source_.size() -3) + "h";
  }  
  // cout << source_ << " and " << header_ << endl;
}

bool ToyNavigator::IsInteresting(CXCursor cursor)
{  
  CXFile file1;
  unsigned line, column, offset;
  clang_getSpellingLocation(clang_getCursorLocation(cursor), 
    &file1, &line, &column, &offset);

  string filename = clang_getCString(clang_getFileName(file1));
  if ((filename.find(source_) != string::npos) || 
      (filename.find(header_) != string::npos)) 
  {
    // cout << "Yes! I'm from " << clang_getCString(clang_getFileName(file1)) 
    //    << " it's " << source_ << endl;
    return true;    
  }
  return false;
}

void PrintSpelling(CXCursor cursor)
{
  cout << clang_getCString(clang_getCursorSpelling(cursor)) << endl;
  // cout << clang_getCString(clang_getCursorDisplayName(cursor)) << endl;
}

string intToString(int x)
{
  stringstream ss;
  ss << x;
  return ss.str();
}

string GetText(CXCursor cursor)
{
  CXFile file1;

  unsigned line, column, offset, stOffs, endOffset;
  clang_getSpellingLocation(clang_getCursorLocation(cursor), &file1, &line, &column, &offset);
  
  CXSourceLocation start = clang_getRangeStart(clang_getCursorExtent(cursor));
  CXSourceLocation end = clang_getRangeEnd(clang_getCursorExtent(cursor));

  clang_getSpellingLocation(start, &file1, &line, &column, &stOffs);
  clang_getSpellingLocation(end, &file1, &line, &column, &endOffset);

  FILE *file;
  char *fn = (char *) clang_getCString(clang_getFileName(file1));
  file = fopen(fn, "r");
  fseek (file, offset, SEEK_SET);
  char *buffer = new char[endOffset -stOffs];

  fread (buffer, 1, (endOffset -stOffs), file);
  fclose (file);

  string str;
  str.assign(buffer);
  delete[] buffer;
  return str;
}

#endif
