// @COMPILE OK
// @EXPECT 100
int main() {
  if (4 > 2) {
    if ( 4 > 5) {
      return 1;
    } else {
      if (8 == 2) return 2;
      return 100;
    }
  } else {
    return 3;
  }
}
