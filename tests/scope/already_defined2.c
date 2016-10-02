// @COMPILE_STATUS 4
int foo;

int func () {
  int foo; // Semantic error
  return 0;
}
