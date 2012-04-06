#ifndef _CONSTANDUTILITIES_H_
#define _CONSTANDUTILITIES_H_

#include "Index.h"

#include <iostream>
#include <stdio.h>

using namespace std;

enum AccessSection { ACCESS_PRIVATE, ACCESS_PUBLIC, ACCESS_PROTECTED, ACCESS_OTHER };
enum FileSection { SECTION_METHOD, SECTION_CLASS, SECTION_OTHER };

void PrintSpelling(CXCursor cursor)
{
  cout << clang_getCString(clang_getCursorSpelling(cursor)) << endl;
  // cout << clang_getCString(clang_getCursorDisplayName(cursor)) << endl;
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
  char *buffer = (char*) malloc (sizeof(char)*(endOffset -stOffs));
  fread (buffer, 1, (endOffset -stOffs), file);

  fclose (file);

  string str;
  str.assign(buffer);
  return str;
}

#endif
