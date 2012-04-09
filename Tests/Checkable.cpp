#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;
//case 1
struct Foo {
    int& m_bar;
    Foo( int bar ): m_bar( bar ) {}
};
//case2
struct Foo1 {
    int m_foo, m_bar;
    Foo1(): m_bar( m_foo ) {}
};


class SomeStuff {
private: 
  virtual void Some() { cout << "some " << endl; };
};

class Checker : public SomeStuff
{
public:
  virtual void Some() { cout << "some another" << endl; };
  void Something() {
    bool a = 5 > 6; //condition that is never true
    int k1 = 56 / 3 + 98;
    int k2 = 345 / k1;

//case 3
    if (k2 > k1 && k2 > k1 && k2 > k1) {
      cout << "hello dead code";
    }
    //case 4
    char *x = "meow";
    int v = abs(strcmp(x, x));

    v = abs(strcmp(x, x));

    size_t i, j;
    for (i = 0; i != 5; i++)
      for (i = 0; i != 5; i++)
        cout << i << j;

    if (false) {
    cout << "";

    } //condition that is never true
        v = abs(strcmp(x, x));
    }

private:
  void AnotherUnusefulMethod();
  static int meow_; //not habdled yet
  void UsefulMethod(); //not handled yet
  void AnotherUsefulMethod(int i) { dummy_ = 5;}; //not handled yet
  void HeyHey() { //unused private method
   cout << "hey hey" << endl; 
   return;
   cout << "meow"; //dead code
  };

public:
  void MeowMeow();
  int Smth();

protected:
private:
  int notDummy_;
  int notDummyButNotEffective_;
  int dummy_;  //unused private field
  int pig_;    //unused private field
};

int Checker::Smth()
{
  cout << "returns nothing";  
  return 0;
}

void Checker::AnotherUnusefulMethod() 
{
  int k = 6;
  int l = k;
  l = 6;
  l = 10;

  if (notDummy_ > 55 - notDummy_) {
    return;
    notDummy_ = 56;
  }
  return;
  notDummy_ = 45;
}

void Checker::UsefulMethod() 
{
  notDummyButNotEffective_ = 4096;
  if (1) {
    notDummy_ = 15;  
  }
  int x = notDummy_ > 5 ? notDummyButNotEffective_ : notDummyButNotEffective_;
  cout << x << endl;

  if (x == 5) {
    cout << "5" << endl;
    if (x == 5)
      cout << "4";
  }
  if (x == 5)
    cout << "r";
  else {
    if (x == 5)
      cout << "78";
  }
  
  UsefulMethod();
  return;
  UsefulMethod();
}

void Checker::MeowMeow()
{
  notDummy_ = 2;

}

class A {
public:
  void print() { /* something */ }
private:
  virtual void func();
};

class B : public A {
public:
  virtual void func();
  int x;
};

class C : public A {
protected:
  virtual void func();
};

void A::func() 
{ 
  cout << "A, private" << endl; 
}

void B::func() 
{ 
  cout << "B, public" << endl; 
}

void C::func() 
{ 
  cout << "xc" << endl; 
}


class K {
public:
  void func(int k, int j);
};

void f()
{
  int k = 456;
  int g = 546-90;

  if (k == g || k == g) {
    cout << "hey" << endl;
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
    cout << "hey" << endl;
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


