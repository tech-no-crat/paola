// @COMPILE OK
// @EXPECT 21
{
  int result;
  int a;
  int b;
  result = 0;

  result = result + (2 == 2);     // 1
  result = result + (2 == 1) * 2; // 0
  a = 981;
  b = 981;
  result = result + (a == b) * 4; // 4
  a = a + b;
  result = result + (a == b) * 8; // 0
  a = a / 2;
  result = result + (a == b) * 16; // 16

  return result;
}
