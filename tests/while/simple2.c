// @COMPILE OK
// @EXPECT 128

{
  int i;
  int x;
  i = 1;
  x = 2;

  while ( i < 7 ) {
    x = x * 2;
    i = i + 1;
  }
  
  return x;
}
