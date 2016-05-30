//@COMPILE OK
//@EXPECT 50
{
  int a;
  int b;
  int c;
  a = 20;
  b = 60;
  c = 90;
  a = 50;
  b = 70;
  c = 160;
  return a;
}

