// @COMPILE OK
// @EXPECT 10
int main() {
  int i;
  int sum;
  sum = 0;

  for (i = 10; i; i = i - 1) {
    sum = sum + 1;
  }

  return sum;
}
