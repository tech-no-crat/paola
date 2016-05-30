//@COMPILE OK
//@EXPECT 48
{
  int a;
  int b;
  int c;

  a = 5;
  b = 10;
  c = 1;

  c = a + b * 2 - c;
  return c * 2;
}
