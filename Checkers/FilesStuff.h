#include <stdio.h>
#include <iostream>
#include <unistd.h> 
#include <dirent.h> 
#include <errno.h>
#include <string>
#include <vector>

using namespace std;

vector<string> GetFileNames(string dir)
{
  vector<string> files;
  DIR *dp;
  struct dirent *dirp;
  if ((dp = opendir(dir.c_str())) == NULL) {
    cout << "Error(" << errno << ") opening " << dir << endl;  
  }

  while ((dirp = readdir(dp)) != NULL) {
    files.push_back(string(dirp->d_name));
  }
  closedir(dp);
  return files;
}

void TestFileNameStuff() 
{
  char buffer[FILENAME_MAX];
  getcwd(buffer, FILENAME_MAX);
  
  string dir = buffer;
  dir.append("/checks/");    
  //current dir is string(".")

  vector<string> files = GetFileNames(dir);
  for (int i = 0; i < files.size(); i++) 
    cout << files[i] << endl;
}
