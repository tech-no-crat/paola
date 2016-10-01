// @COMPILE OK
// @EXPECT 51

// Returns 16
int bar() {
  int x;
  int y;
  x = 8;
  y = 2;
  return x * y;
}

// Returns 98
int foo() {
  int a;
  int b;
  a = 10;
  b = 5;
  a = a * b; // 50
  a = a + bar(); // 66
  return a + bar() * 2; //98
}

int main() {
  int res;
  res = foo() / 2;
  return res + 2; // 51
}
