#include <iostream>
#include <stdio.h>

using namespace std;

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
    if (a) {
      cout << "hello dead code";
    }
    int k;

    if (false) {
    cout << "";
    } //condition that is never true
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

protected:
private:
  int notDummy_;
  int notDummyButNotEffective_;
  int dummy_;  //unused private field
  int pig_;    //unused private field
};

void Checker::AnotherUnusefulMethod() 
{
  int k = 6;
  int l = k;
  l = 6;
  l = 10;

  if (56 > 55) {
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
  
  UsefulMethod();
  return;
  UsefulMethod();
}

void Checker::MeowMeow()
{
  notDummy_ = 2;
}
