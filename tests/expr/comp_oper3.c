// @COMPILE OK
// @EXPECT 1
int main() {
  int result;
  int a;
  int b;
  int pass;
  int fail;
  pass = 1;
  fail = 0;
  result = 0;

  a = 11;
  b = 12;
  result = result + (a < b); // 1
  result = result + (b < a) * 2; // 0
  result = result + (a <= b) * 4; // 4
  result = result + (b <= a) * 8;  // 0
  result = result + (a > b) * 16; // 0
  result = result + (b > a) * 32; // 32
  result = result + (a >= b) * 64; // 0
  result = result + (b >= a) * 128; // 128

  if (result == 165) return pass;
  else return fail;
}
