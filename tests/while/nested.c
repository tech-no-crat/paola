// @COMPILE OK
// @EXPECT 250

int main() {
  int i;
  int y;
  y = 0;
  i = 5;
  while ( i ){
    int j;
    int x;
    j = 10;
    x = 0;
    while (j) {
      x = x + 5;
      j = j - 1;
    }

    y = y + x;
    i = i - 1;
  }

  return y;
}
