// @COMPILE OK
// @EXPECT 42

int foo() {
  int x;
  int y;
  int z;

  x = 20;
  y = 20;
  z = 1;
  z = x + y + z;
  return z;
}

int main() {
  int x;
  int a;
  x = foo();
  return foo() + 1;
}
