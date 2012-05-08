#include <iostream>
#include <stdio.h>
#include <string.h>

#include "Checkable.h"

int Checker::Smth()
{
  cout << "returns nothing";  
  return 0;
}

class KL {
private:
  int x;
};

// void Checker::AnotherUnusefulMethod() 
// {
//   int k = 6;
//   int l = k;
//   l = 6;
//   l = 10;

//   if (notDummy_ > 55 - notDummy_) {
//     return;
//     notDummy_ = 56;
//   }
//   return;
//   notDummy_ = 45;
// }

void Checker::UsefulMethod() 
{
  // notDummyButNotEffective_ = 4096;
  // if (1) {
  //   notDummy_ = 15;  
  // }
  int x = notDummy_ > 5 ? notDummyButNotEffective_ : notDummyButNotEffective_;
  // cout << x << endl;

  if (x == 5 || x < 10 *x) {
    cout << "5" << endl;
    if (x == 5 || x > 54)
      cout << "4";
    if (x == 7 || x > 54)
      cout << "4";
    if (x == 5 || x > 54)
      cout << "4";
    if (x == 5 || x > 54)
      cout << "4";
  }
  if (x == 5 || x < 10 *x)
    cout << "r";
  else {
    if (x == 5)
      cout << "78";
  }
  
  // UsefulMethod();
  return;
  // UsefulMethod();
}

// void Checker::MeowMeow()
// {
//   notDummy_ = 2;

// }

// void A::func() 
// { 
//   cout << "A, private" << endl; 
// }

// void B::func() 
// { 
//   cout << "B, public" << endl; 
// }

// void C::func() 
// { 
//   cout << "xc" << endl; 
// }


void f()
{
  int k = 456;
  int g = 546-90;

  if (k == g || k == g) {
    int h = k - 50 + 97 *g;
    if (h < k && h > k && h < k)
      cout << "h" << endl;
  } 
  for (int i = 0; i < 1; i++) {
    cout << "h";
  }
}

void K::func(int k, int j) 
{
  if (k == j || k == j || k != j || k == j) {
    int h = k - 50 + 97 *j;
    if (h < k && h > k && h < k)
      cout << "h" << endl;
  }
  if ( k < 967*j)
    cout << "fgdfg" << endl;
  int k1;
  while (k1 < 5 || k1 < 5) {
    k1 = 5;
  }
}


