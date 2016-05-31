// @COMPILE OK
// @EXPECT 50

{
  int i;
  int x;
  i = 10;
  x = 0;
  while ( i ) {
    x = x + 5;
    i = i - 1;
  }
  return x;
}
