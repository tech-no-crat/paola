// @COMPILE OK
// @EXPECT 42
int main () {

  if (2==1);
  else {
    if (2==1) {}
    else {
      return 42;
    }
  }

  return 0;
}
