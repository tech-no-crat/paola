// @COMPILE OK
// @EXPECT 57
int main() {
  int result;
  result = 0;

  result = result + (8 < 12); // 1
  result = result + (12 < 8) * 2; // 0
  result = result + (9 < 9) * 4; // 0
  result = result + (9 <= 9) * 8; // 8
  result = result + (9 >= 9) * 16; // 16
  result = result + (12 > 8) * 32; // 32
  result = result + (8 > 12) * 64; // 0
  result = result + (9 > 9) * 128; // 0

  return result;
}
