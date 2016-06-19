// @COMPILE OK
// @EXPECT 120
// Computes 5! = 120
{
  int i;
  int result;

  i = 42;
  result = 1;
  for (i = 5; i; i = i - 1) {
     if (i) result = result * i;
  }

  return result;
}
