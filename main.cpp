#include "Index.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include <list>

#include "Checkers/IChecker.h"

#include "Checkers/UnusedMembersChecker.h"
#include "Checkers/AccessLevelChecker.h"
#include "Checkers/DeadCodeChecker.h"
#include "Checkers/SameConditionsChecker.h"
#include "Checkers/ConditionChecker.h"


#include "Checkers/ConstAndUtilities.h"

using namespace std;

class Runner 
{
public:
  static IChecker *checker;
  static CXCursor cursor;
  static std::vector<IChecker*> checkers;

  static void RunAll()
  {
    CXClientData data;
    for (int i = 0; i < checkers.size(); i++) {
      Runner::checker = checkers[i];
      // cout << "next checker: " << checker->GetName() << endl;
      Runner::Run(cursor, &data);
    }
  }
  static void Run(CXCursor cursor, CXClientData client_data) 
  {
    // cout << "running checker..." << endl;
    if (!checker) {
      cout << "the checker is null" << endl;
      return;
    }

    // cout << "resetting..." << endl;
    checker->Reset();
    // cout << "visit_" << endl;
    clang_visitChildren(cursor, Runner::Check, &client_data);
    // cout << "showing diag" << endl;
    cout << /*FormatDiag(checker->GetDiagnostics()) <<*/ checker->GetStatistics() << endl;
  }

  static enum CXChildVisitResult Check(CXCursor cursor,
    CXCursor parent, CXClientData client_data)
  {
    if (clang_getCursorKind(cursor) == CXCursor_NullStmt) {    
      return CXChildVisit_Break;
    }
    if (!checker) {
      cout << "Check: the checker is null" << endl;
      return CXChildVisit_Break;
    }

    std::vector<CXCursorKind> cursors = checker->GetInterestingCursors();
    if (std::find(cursors.begin(), cursors.end(), clang_getCursorKind(cursor)) 
      != cursors.end()) 
    {
      if (ToyNavigator::IsInteresting(cursor)) {
        checker->Check(cursor, parent, client_data);
      }
    }
    return CXChildVisit_Continue;
  }
};

IChecker* Runner::checker = NULL;
std::vector<IChecker*> Runner::checkers = std::vector<IChecker*>();
CXCursor Runner::cursor = clang_getNullCursor();

void InitRunner()
{
   IChecker* umc = new UnusedMembersChecker;
  Runner::checkers.push_back(umc);

  IChecker* alc = new AccessLevelChecker;
  Runner::checkers.push_back(alc);

  IChecker* dcc = new DeadCodeChecker;
  Runner::checkers.push_back(dcc);

  // IChecker* cc = new ConditionChecker;
  // Runner::checkers.push_back(cc);

  // IChecker* scc = new SameConditionsChecker;
  // Runner::checkers.push_back(scc);
}

void ProcessFile(int argc, char* argv[], char* filename)
{
  int k = 0;
  while (argv[k] != NULL) {
    k++; //getting real number
  }

  CXIndex index = clang_createIndex(1, 0);  
  CXTranslationUnit tUnit = clang_parseTranslationUnit(index, filename, argv, 
      k, 0, 0, CXTranslationUnit_None);

  ToyNavigator::SetFile(filename);

  for (int i = 0, n = clang_getNumDiagnostics(tUnit); i != n; i++) {
    CXDiagnostic diag = clang_getDiagnostic(tUnit, i);
    CXString str = clang_formatDiagnostic(diag, 
        clang_defaultDiagnosticDisplayOptions());
    cout << clang_getCString(str) << endl;
    clang_disposeString(str);
  }

  Runner::cursor = clang_getTranslationUnitCursor(tUnit);
  Runner::RunAll();

  clang_disposeTranslationUnit(tUnit); 
  clang_disposeIndex(index);
}

int main(int argc, char* argv[])
{  
  char* files[argc];
  char* args[argc];

  int j = 0;
  int i = 0;

  while (strncmp(argv[i], "$", 5)) {
    files[j++] = argv[i++];
  }
  files[j] = NULL;

  i++;
  int k = 0;
  // args[0] = argv[0];
  while (argv[i] != NULL) {
    args[k++] = argv[i++];    
  }
  args[k] = NULL;

  InitRunner();
  cout << "j: " << j << " i: " << i << endl;

  for (int l = 1; l < j; l++) {
    cout << "Processing file...( " << l << " / " << j << " )" << files[l] << endl;
    ProcessFile(k +1, args, files[l]);
    cout << "Processed: ( " << l << " / " << j << " )" << endl;
  }

  /*else {
    cout << "No diagnostics available for " << str << endl;
  }*/

  return 0;
}