// @COMPILE OK
// @EXPECT 130

int foo() {
  int a;
  int b;
  int c;
  int d;
  
  a = 10; b = 20; c = 30;
  d = a + c;
  a = a + b + c + d; // unused
  return d; // 40
}

int main() {
  int x;
  int y;
  
  x = 10;
  y = x + foo();


  return y + foo() * 2;
}
